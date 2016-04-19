
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace tab {

struct ThreadedSeqFile : public obj::SeqBase {

    obj::String* holder() {
        static thread_local obj::String ret;
        return &ret;
    }

    std::mutex mutex;
    funcs::Linereader reader;
    
    ThreadedSeqFile(std::istream& infile) : reader(infile) {}

    obj::Object* next() {
        std::lock_guard<std::mutex> l(mutex);

        bool ok = reader.getline(holder()->v);

        if (!ok) return nullptr;

        return holder();
    }
};

struct ThreadGroupSeq : public obj::SeqBase {

    struct syncvar_t {
        obj::Object* result;
        std::condition_variable can_produce;
        std::mutex mutex;
        bool finished;

        syncvar_t() : result(nullptr), finished(false) {}
    };

    std::vector< std::unique_ptr<syncvar_t> > syncs;
    std::vector<syncvar_t*> queued;
    std::vector<std::thread> threads;
    size_t nthreads;
    ssize_t last_used_thread;

    template <typename API, typename T>
    void threadfun(API& api, T& code, obj::Object*& seq, obj::Object* input, syncvar_t* sync) {

        {
            std::unique_lock<std::mutex> l(sync->mutex);

            obj::Object* r = api.run(code, input);

            if (seq == nullptr) {
                seq = r;

            } else {
                seq->wrap(r);
            }
        }

        while (1) {
            std::unique_lock<std::mutex> l(sync->mutex);

            while (sync->result) {
                sync->can_produce.wait(l);
            }

            sync->result = seq->next();

            if (!sync->result) {

                sync->finished = true;
                break;
            }
        }
    }

    template <typename API, typename T>
    ThreadGroupSeq(API& api, std::vector<T>& codes, std::vector<obj::Object*>& seqs, obj::Object* input) :
        nthreads(codes.size()), last_used_thread(-1) {

        syncs.resize(nthreads);
        queued.resize(nthreads);

        for (size_t i = 0; i < nthreads; ++i) {
            syncs[i].reset(new syncvar_t);
            queued[i] = syncs[i].get();
        }

        for (size_t i = 0; i < nthreads; ++i) {
            threads.emplace_back(&ThreadGroupSeq::threadfun<API, T>, this,
                                 std::ref(api), std::ref(codes[i]), std::ref(seqs[i]), input, queued[i]);
        }
    }

    ~ThreadGroupSeq() {
        for (auto& t : threads) {
            t.join();
        }
    }

    obj::Object* next() {

        if (last_used_thread >= 0) {
            syncvar_t* luts = queued[last_used_thread];

            luts->result = nullptr;
            luts->mutex.unlock();
            luts->can_produce.notify_one();
        }

        size_t n = last_used_thread;

        while (1) {
            ++n;
            n = n % nthreads;

            syncvar_t* sync = queued[n];

            sync->mutex.lock();

            if (!sync->finished && !sync->result) {
                sync->mutex.unlock();
                continue;
            }

            if (sync->finished) {
                sync->mutex.unlock();
                last_used_thread = -1;
                --nthreads;

                queued.erase(queued.begin() + n);

                if (queued.empty())
                    return nullptr;

                return next();
            }

            last_used_thread = n;
                
            return sync->result;
        }
    }

};

}

template <bool SORTED>
void run_threaded(size_t seed, const std::string& program, size_t nthreads, 
                  const std::string& infile, unsigned int debuglevel) {

    if (nthreads == 0)
        nthreads = 1;

    std::string scatter;
    std::string gather;

    {
        size_t i = program.find("-->");

        if (i == std::string::npos) {
            scatter = program;
            gather = "@";

        } else {
            scatter = program.substr(0, i);
            gather = program.substr(i + 3);
        }
    }

    tab::API<SORTED> api;

    api.init(seed);

    static const tab::Type intype(tab::Type::SEQ, { tab::Type(tab::Type::STRING) });

    typedef typename tab::API<SORTED>::compiled_t compiled_t;

    tab::obj::Object* input = new tab::ThreadedSeqFile(file_or_stdin(infile));

    std::vector<compiled_t> codes;
    std::vector<tab::obj::Object*> seqs;

    codes.resize(nthreads);
    seqs.resize(nthreads);

    for (size_t n = 0; n < nthreads; ++n) {

        auto& code = codes[n];

        api.compile(scatter.begin(), scatter.end(), intype, code, (n == 0 ? debuglevel : 0));

        seqs[n] = (tab::functions().seqmaker)(code.result);

        if (seqs[n]) {
            code.result = tab::wrap_seq(code.result);
        }
    }

    compiled_t gathered;
    api.compile(gather.begin(), gather.end(), codes[0].result, gathered, debuglevel);

    tab::ThreadGroupSeq* tgs = new tab::ThreadGroupSeq(api, codes, seqs, input);

    tab::obj::Object* output = api.run(gathered, tgs);

    tab::obj::Printer p;
    output->print(p);
    p.nl();

    delete tgs;
}
