
#include <thread>
#include <mutex>

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

    static tab::Type intype(tab::Type::SEQ, { tab::Type(tab::Type::STRING) });

    typename tab::API<SORTED>::compiled_t code;
    api.compile(program.begin(), program.end(), intype, code, debuglevel);

    tab::obj::Object* input = new tab::funcs::SeqFile(file_or_stdin(infile));
    tab::obj::Object* output = api.run(code, input);

    tab::obj::Printer p;
    output->print(p);
    p.nl();
}

struct ThreadedSeqFile : public tab::obj::SeqBase {

    tab::obj::String* holder() {
        static thread_local tab::obj::String ret;
        return &ret;
    }

    std::mutex mutex;
    tab::funcs::Linereader reader;
    
    ThreadedSeqFile(std::istream& infile) : reader(infile) {}

    tab::obj::Object* next() {
        std::lock_guard<std::mutex> l(mutex);

        bool ok = reader.getline(holder()->v);

        if (!ok) return nullptr;

        return holder();
    }
};

struct ThreadedPrinter : public tab::obj::Printer {

    std::string& buff() {
        static thread_local std::string ret;
        return ret;
    }

    virtual void val(tab::UInt v) { buff() += std::to_string(v); }
    virtual void val(tab::Int v)  { buff() += std::to_string(v); }
    virtual void val(tab::Real v) { buff() += std::to_string(v); }

    virtual void val(const std::string& v) { buff() += v; }

    virtual void rs() { buff() += '\t'; }

    virtual void nl() {
        std::string& b = buff();
        b += '\n';
        fwrite(b.data(), sizeof(char), b.size(), stdout);
        b.clear();
    }
};


template <bool SORTED>
void run_threaded(size_t seed, const std::string& program, const std::string& infile, size_t nthreads, unsigned int debuglevel) {

    tab::API<SORTED> api;

    api.init(seed);

    static tab::Type intype(tab::Type::SEQ, { tab::Type(tab::Type::STRING) });

    typedef typename tab::API<SORTED>::compiled_t compiled_t;

    std::vector<compiled_t> codes;
    std::vector<std::thread> threads;

    tab::obj::Object* input = new ThreadedSeqFile(file_or_stdin(infile));

    ThreadedPrinter printer;

    auto threadfun = [&](compiled_t& c) {

        tab::obj::Object* output = api.run(c, input);
        output->print(printer);
        printer.nl();
    };

    codes.resize(nthreads);

    for (size_t nt = 0; nt < nthreads; ++nt) {
        api.compile(program.begin(), program.end(), intype, codes[nt], debuglevel);
    }

    for (size_t nt = 0; nt < nthreads; ++nt) {
        threads.emplace_back(threadfun, std::ref(codes[nt]));
    }

    for (auto& t : threads) {
        t.join();
    }
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
              << "  -v:   verbosity flag -- print type of the result." << std::endl
              << "  -vv:  verbosity flag -- print type of the result and VM instructions." << std::endl
              << "  -vvv: verbosity flag -- print type of the result, VM instructions and parse tree." << std::endl
              << "  -h:   show help from given section." << std::endl
              << std::endl
              << "Help sections:" << std::endl
              << "  'overview'      -- show an overview of types and concepts." << std::endl
              << "  'syntax'        -- show a syntax reference." << std::endl
              << "  'examples'      -- show some example tab programs." << std::endl
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
                run_threaded<true>(seed, program, infile, nthreads, debuglevel);
            } else {
                run<true>(seed, program, infile, debuglevel);
            }

        } else {

            if (nthreads > 0) {
                run_threaded<false>(seed, program, infile, nthreads, debuglevel);
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
