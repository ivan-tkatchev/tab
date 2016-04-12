
#include <thread>
#include <mutex>
#include <condition_variable>

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
        std::unique_ptr<std::condition_variable> can_produce;
        std::unique_ptr<std::mutex> mutex;

        syncvar_t() : result(nullptr), can_produce(new std::condition_variable), mutex(new std::mutex) {}
    };

    std::vector<syncvar_t> syncs;
    std::vector<std::thread> threads;
    size_t nthreads;
    ssize_t last_used_thread;

    std::mutex mutex;
    size_t nidle;
    size_t nfinished;
    std::condition_variable can_consume;

    template <typename API, typename T>
    void threadfun(API& api, T& code, obj::Object*& seq, obj::Object* input, size_t n) {

        obj::Object* r = api.run(code, input);

        if (seq == nullptr) {
            seq = r;

        } else {
            seq->wrap(r);
        }

        auto& sync = syncs[n];

        while (1) {

            std::unique_lock<std::mutex> l(*(sync.mutex));

            while (sync.result) {
                sync.can_produce->wait(l);
            }

            {
                std::unique_lock<std::mutex> ll(mutex);
               --nidle;
            }

            obj::Object* next = seq->next();

            sync.result = next;

            if (!next) {

                std::unique_lock<std::mutex> ll(mutex);
                ++nfinished;
                ll.unlock();
                can_consume.notify_one();
                break;

            } else {

                std::unique_lock<std::mutex> ll(mutex);
                ++nidle;
                ll.unlock();
                can_consume.notify_one();
            }
        }
    }

    template <typename API, typename T>
    ThreadGroupSeq(API& api, std::vector<T>& codes, std::vector<obj::Object*>& seqs, obj::Object* input) :
        nthreads(codes.size()), last_used_thread(-1), nidle(nthreads), nfinished(0) {

        syncs.resize(nthreads);

        for (size_t i = 0; i < nthreads; ++i) {
            threads.emplace_back(&ThreadGroupSeq::threadfun<API, T>, this,
                                 std::ref(api), std::ref(codes[i]), std::ref(seqs[i]), input, i);
        }
    }

    ~ThreadGroupSeq() {
        for (auto& t : threads) {
            t.join();
        }
    }

    obj::Object* next() {

        if (last_used_thread >= 0) {
            auto& luts = syncs[last_used_thread];

            luts.result = nullptr;
            luts.mutex->unlock();
            luts.can_produce->notify_one();
        }

        while (1) {

            int n = -1;
            for (auto& i : syncs) {
                ++n;

                auto& m = *(i.mutex);

                if (!m.try_lock())
                    continue;

                if (i.result == nullptr) {
                    m.unlock();
                    continue;
                }

                last_used_thread = n;
                
                return i.result;
            }

            std::unique_lock<std::mutex> l(mutex);

            if (nfinished == nthreads) return nullptr;

            if (nidle == 0) {
                can_consume.wait(l);
            }

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
