
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

    std::mutex mutex;
    std::condition_variable can_consume;

    struct syncvar_t {
        bool produced;
        bool finished;

        size_t serial;
        obj::Object* result;
        std::unique_ptr<std::condition_variable> can_produce;
        std::unique_ptr<std::mutex> mutex;

        syncvar_t() : produced(false), finished(false), serial(0), result(nullptr),
                      can_produce(new std::condition_variable), mutex(new std::mutex) {}
    };

    std::vector<syncvar_t> syncs;
    std::vector<std::thread> threads;
    size_t serial;

    void threadfun(obj::Object* seq, size_t n) {

        auto& sync = syncs[n];

        while (1) {

            std::unique_lock<std::mutex> l(*(sync.mutex));

            size_t s = sync.serial;

            while (sync.produced || s == sync.serial) {
                sync.can_produce->wait(l);
            }

            obj::Object* next = seq->next();

            if (next) {
                tprinter().p("sent :: ");
                tprinter().p((int)n);
                tprinter().p(" ");
                tprinter().p((int)sync.serial);
                tprinter().p(" ");
                tprinter().p((int)sync.produced);
                tprinter().p(" {");
                tprinter().p(obj::get<obj::String>(next).v);
                tprinter().p("}\n");
            }

            sync.produced = true;
            sync.result = next;

            if (!next) {
                sync.finished = true;
            }

            l.unlock();
            can_consume.notify_one();

            if (!next) break;
        }
    }

    ThreadGroupSeq(std::vector<obj::Object*> seqs) : serial(0) {

        size_t nthreads = seqs.size();
        syncs.resize(nthreads);

        for (size_t i = 0; i < nthreads; ++i) {
            threads.emplace_back(&ThreadGroupSeq::threadfun, this, seqs[i], i);
        }
    }

    ~ThreadGroupSeq() {
        for (auto& t : threads) {
            t.join();
        }
    }

    obj::Object* next() {

        ++serial;

        while (1) {
            bool all_done = true;

            int n = -1;
            for (auto& i : syncs) {
                ++n;

                std::unique_lock<std::mutex> l(*(i.mutex));

                if (i.finished) continue;

                all_done = false;

                if (i.produced) {
                    i.produced = false;
                    i.serial = serial;

                    if (i.result) {
                        tprinter().p("got :: ");
                        tprinter().p((int)n);
                        tprinter().p(" ");
                        tprinter().p((int)i.serial);
                        tprinter().p(" ");
                        tprinter().p((int)i.produced);
                        tprinter().p(" {");
                        tprinter().p(obj::get<obj::String>(i.result).v);
                        tprinter().p("}\n");
                    }

                    return i.result;

                } else if (i.serial < serial) {
                    i.serial = serial;
                    l.unlock();
                    i.can_produce->notify_one();
                }
            }

            if (all_done) return nullptr;

            std::unique_lock<std::mutex> l(mutex);
            can_consume.wait(l);
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

    std::vector<tab::obj::Object*> scattered;
    std::vector<compiled_t> codes;

    for (size_t nt = 0; nt < nthreads; ++nt) {

        codes.emplace_back();
        compiled_t& code = codes.back();

        api.compile(scatter.begin(), scatter.end(), intype, code, debuglevel);

        tab::obj::Object* r = api.run(code, input);

        tab::obj::Object* s = (tab::functions().seqmaker)(code.result);

        if (s == nullptr) {
            s = r;

        } else {
            code.result = tab::wrap_seq(code.result);
            s->wrap(r);
        }

        scattered.push_back(s);
    }

    tab::ThreadGroupSeq* tgs = new tab::ThreadGroupSeq(scattered);

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
