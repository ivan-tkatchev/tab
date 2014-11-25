
#include "tab.h"

int main(int argc, char** argv) {

    try {

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <expression>" << std::endl;
            return 1;
        }

        std::string program(argv[1]);

        register_functions();

        std::vector<Command> commands;
        TypeRuntime typer;

        Type finaltype = parse(program.begin(), program.end(), typer, commands);

        execute(commands, finaltype, std::cin);
        
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
