
#include "tab.h"

#include <time.h>

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

    API<SORTED> api;

    api.init(seed);

    static Type intype(Type::SEQ, { Type(Type::STRING) });

    typename API<SORTED>::compiled_t code;
    api.compile(program.begin(), program.end(), intype, code, debuglevel);

    obj::Object* input = new funcs::SeqFile(file_or_stdin(infile));
    obj::Object* output = api.run(code, input);

    output->print();
    std::cout << std::endl;
}


int main(int argc, char** argv) {

    try {

        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <expression>" << std::endl;
            return 1;
        }

        unsigned int debuglevel = 0;
        bool sorted = false;
        std::string program;
        std::string infile;
        std::string programfile;
        size_t seed = ::time(NULL);

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

                std::cout <<
                    "Usage: tab [-i inputdata_file] [-f expression_file] [-r random seed] [-s] [-v|-vv|-vvv] <expressions...>"
                          << std::endl
                          << "  -i:   read data from this file instead of stdin." << std::endl
                          << "  -f:   prepend code from this file to <expressions...>" << std::endl
                          << "  -r:   use a specific random seed." << std::endl
                          << "  -s:   use maps with keys in sorted order instead of the unsorted default." << std::endl
                          << "  -v:   verbosity flag -- print type of the result." << std::endl
                          << "  -vv:  verbosity flag -- print type of the result and VM instructions." << std::endl
                          << "  -vvv: verbosity flag -- print type of the result, VM instructions and parse tree." 
                          << std::endl;
                return 1;
                
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
