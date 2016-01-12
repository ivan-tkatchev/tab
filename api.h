#ifndef __TAB_API_H
#define __TAB_API_H

template <bool SORTED>
struct TheRuntime {

    static void init(size_t seed) {

        register_functions<SORTED>(seed);
    }

    struct compiled_t {
        std::vector<Command> commands;
        size_t num_vars;
        Type result;
    };

    template <typename I>
    static void compile(I beg, I end, const Type& input, compiled_t& out, unsigned int debuglevel = 0) {

        TypeRuntime typer;
        out.result = parse(beg, end, input, typer, out.commands, debuglevel);
        out.num_vars = typer.num_vars();
    }

    static void run(compiled_t& code, std::istream& inputs) {

        execute<SORTED>(code.commands, code.result, code.num_vars, inputs);
    }
};


#endif
