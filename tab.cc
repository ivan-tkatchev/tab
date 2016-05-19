
#include <time.h>

#include "tab.h"


std::istream& file_or_stdin(const std::string& file) {

    if (file.empty())
        return std::cin;

    static std::ifstream ret;

    ret.open(file);

    if (!ret)
        throw std::runtime_error("Could not open input file: " + file);

    return ret;
}        

#ifdef _REENTRANT
#include "threaded.h"
#endif


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

extern const char* get_help(const std::string&);

void show_help(const std::string& help_section) {

    const char* help = get_help(help_section);

    if (help) {
        std::cout << help << std::endl;
        return;
    }

    std::cout <<
        "Usage: tab [-i inputdata_file] [-f expression_file] [-t N] [-r random seed] [-s] [-v|-vv|-vvv] [-h section] "
              << "<expressions...>"
              << std::endl
              << "  -i:   read data from this file instead of stdin." << std::endl
              << "  -f:   prepend code from this file to <expressions...>" << std::endl
              << "  -r:   use a specific random seed." << std::endl
              << "  -s:   use maps with keys in sorted order instead of the unsorted default." << std::endl
#ifdef _REENTRANT
              << "  -t:   use N parallel threads for evaluating the expression." << std::endl
#endif
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

bool getopt(unsigned char opt, int argc, char** argv, int& i, std::string& out, bool required = true) {

    if (argv[i][0] == '-' && argv[i][1] == opt) {

        if (argv[i][2] == '\0') {

            if (i == argc - 1 || argv[i + 1][0] == '-') {

                if (!required) return true;

                std::runtime_error("The '-" + std::string(opt, 1) +"' command line argument expects an argument.");
            }

            ++i;
            out = argv[i];

        } else {

            out = argv[i] + 2;
        }

        return true;
    }

    return false;
}

int main(int argc, char** argv) {
    try {

        if (argc < 2) {
            show_help("");
            return 1;
        }

        unsigned int debuglevel = 0;
        bool sorted = false;
        std::string program;
        std::string infile;
        std::string programfile;
        size_t seed = ::time(NULL);
        bool help = false;
        std::string help_section;

        size_t nthreads = 0;

        for (int i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            std::string out;

            if (arg == "-v" || getopt('v', argc, argv, i, out, false)) {

                if (out == "vv") {
                    debuglevel = 3;
                } else if (out == "v") {
                    debuglevel = 2;
                } else {
                    debuglevel = 1;
                }

            } else if (getopt('r', argc, argv, i, out)) {

                seed = std::stoul(out);

            } else if (arg == "-s") {

                sorted = true;

            } else if (getopt('f', argc, argv, i, programfile)) {
                //

            } else if (getopt('i', argc, argv, i, infile)) {
                //

            } else if (getopt('h', argc, argv, i, help_section, false)) {

#ifdef _REENTRANT
            } else if (getopt('t', argc, argv, i, out)) {

                nthreads = std::stoul(out);
#endif
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
                program = program + "," + programfile;
            }
        }

        // //

        if (help || program.empty()) {
            show_help(help_section);
            return 1;
        }

        // //

#ifdef _REENTRANT
        if (nthreads > 0) {
            if (sorted) {
                run_threaded<true>(seed, program, nthreads, infile, debuglevel);
            } else {
                run_threaded<false>(seed, program, nthreads, infile, debuglevel);
            }

            return 0;
        }
#endif
            
        if (sorted) {
            run<true>(seed, program, infile, debuglevel);
        } else {
            run<false>(seed, program, infile, debuglevel);
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
