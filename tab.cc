
#include <thread>
#include <mutex>
#include <condition_variable>

#include "tab.h"

#include <time.h>

extern const char* get_help(const char*);

std::istream& file_or_stdin(const std::string& file) {

    if (file.empty())
        return std::cin;

    static std::ifstream ret;

    ret.open(file);

    if (!ret)
        throw std::runtime_error("Could not open input file: " + file);

    return ret;
}        

template <bool SORTED>
void run(size_t seed, const std::string& program, const std::string& infile, unsigned int debuglevel) {

    tab::API<SORTED> api;

    api.init(seed);

    static const tab::Type intype(tab::Type::SEQ, { tab::Type(tab::Type::STRING) });

    typename tab::API<SORTED>::compiled_t code;
    api.compile(program.begin(), program.end(), intype, code, debuglevel);

    tab::obj::Object* input = new tab::funcs::SeqFile(file_or_stdin(infile));
    tab::obj::Object* output = api.run(code, input);

    tab::obj::Printer p;
    output->print(p);
    p.nl();
}

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

struct threadedprinter {

    std::string r;
    std::mutex mutex;

    void p(int v) { std::unique_lock<std::mutex> l(mutex); r += std::to_string(v); }
    void p(const std::string& s) { std::unique_lock<std::mutex> l(mutex); r += s; }

    ~threadedprinter() {
        std::unique_lock<std::mutex> l(mutex);
        printf("%s\n", r.c_str());
    }
};

threadedprinter& tprinter() {
    static threadedprinter ret;
    return ret;
}

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

        api.compile(scatter.begin(), scatter.end(), intype, code, debuglevel);

        seqs[n] = (tab::functions().seqmaker)(code.result);

        if (seqs[n]) {
            code.result = tab::wrap_seq(code.result);
        }
    }

    tab::ThreadGroupSeq* tgs = new tab::ThreadGroupSeq(api, codes, seqs, input);

    compiled_t gathered;
    api.compile(gather.begin(), gather.end(), codes[0].result, gathered, debuglevel);

    tab::obj::Object* output = api.run(gathered, tgs);

    tab::obj::Printer p;
    output->print(p);
    p.nl();

    delete tgs;
}


void show_help(const char* help_section) {

    const char* help = get_help(help_section);

    if (help) {
        std::cout << help << std::endl;
        return;
    }

    std::cout <<
        "Usage: tab [-i inputdata_file] [-f expression_file] [-r random seed] [-s] [-v|-vv|-vvv] [-h section] "
              << "<expressions...>"
              << std::endl
              << "  -i:   read data from this file instead of stdin." << std::endl
              << "  -f:   prepend code from this file to <expressions...>" << std::endl
              << "  -r:   use a specific random seed." << std::endl
              << "  -s:   use maps with keys in sorted order instead of the unsorted default." << std::endl
              << "  -t N: use N parallel threads for evaluating the expression." << std::endl
              << "        (use '-->' to separate scatter and gather subexpressions; see tab -h 'threads')" << std::endl
              << "  -v:   verbosity flag -- print type of the result." << std::endl
              << "  -vv:  verbosity flag -- print type of the result and VM instructions." << std::endl
              << "  -vvv: verbosity flag -- print type of the result, VM instructions and parse tree." << std::endl
              << "  -h:   show help from given section." << std::endl
              << std::endl
              << "Help sections:" << std::endl
              << "  'overview'      -- show an overview of types and concepts." << std::endl
              << "  'syntax'        -- show a syntax reference." << std::endl
              << "  'examples'      -- show some example tab programs." << std::endl
              << "  'threads'       -- show examples of multithreaded programs." << std::endl
              << "  'functions'     -- show a complete list of built-in functions." << std::endl
              << "  <function name> -- explain the given built-in function." << std::endl;
}

int main(int argc, char** argv) {
    try {

        if (argc < 2) {
            show_help(nullptr);
            return 1;
        }

        unsigned int debuglevel = 0;
        bool sorted = false;
        std::string program;
        std::string infile;
        std::string programfile;
        size_t seed = ::time(NULL);
        bool help = false;
        const char* help_section = nullptr;

        size_t nthreads = 0;

        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);

            if (arg == "-v") {
                debuglevel = 1;

            } else if (arg == "-vv") {
                debuglevel = 2;

            } else if (arg == "-vvv") {
                debuglevel = 3;

            } else if (arg == "-r") {

                if (i == argc - 1)
                    throw std::runtime_error("The '-r' command line argument expects a numeric argument.");

                ++i;
                seed = std::stoul(argv[i]);

            } else if (arg == "-s") {

                sorted = true;

            } else if (arg == "-f") {

                if (i == argc - 1)
                    throw std::runtime_error("The '-f' command line argument expects a filename argument.");

                ++i;
                programfile = argv[i];

            } else if (arg == "-i") {

                if (i == argc - 1)
                    throw std::runtime_error("The '-i' command line argument expects a filename argument.");

                ++i;
                infile = argv[i];

            } else if (arg == "-h") {

                help = true;

                if (i < argc - 1) {
                    ++i;
                    help_section = argv[i];
                }

            } else if (arg == "-t") {

                if (i < argc - 1) {
                    ++i;
                    nthreads = std::stoul(argv[i]);
                }

            } else {

                if (program.size() > 0) {
                    program += ' ';
                }

                program += arg;
            }
        }

        if (programfile.size() > 0) {

            std::ifstream f;
            f.open(programfile);

            if (!f)
                throw std::runtime_error("Could not open input program file: " + programfile);

            programfile.assign(std::istreambuf_iterator<char>(f),
                               std::istreambuf_iterator<char>());

            if (program.empty()) {
                program = programfile;
            } else {
                program = programfile + "," + program;
            }
        }

        // //

        if (help || program.empty()) {
            show_help(help_section);
            return 1;
        }

        // //

        if (sorted) {

            if (nthreads > 0) {
                run_threaded<true>(seed, program, nthreads, infile, debuglevel);
            } else {
                run<true>(seed, program, infile, debuglevel);
            }

        } else {

            if (nthreads > 0) {
                run_threaded<false>(seed, program, nthreads, infile, debuglevel);
            } else {
                run<false>(seed, program, infile, debuglevel);
            }
        }
        
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
