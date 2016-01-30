#ifndef __TAB_API_H
#define __TAB_API_H

namespace tab {

template <bool SORTED>
struct API {

    static void init(size_t seed) {

        register_functions<SORTED>(seed);
    }

    struct compiled_t {
        std::vector<Command> commands;
        Type result;
        Runtime rt;
    };

    template <typename I>
    static void compile(I beg, I end, const Type& input, compiled_t& out, unsigned int debuglevel = 0) {

        TypeRuntime typer;
        out.result = parse(beg, end, input, typer, out.commands, debuglevel);

        out.rt.init(typer.num_vars());

        execute_init<SORTED>(out.commands);
    }

    static obj::Object* run(compiled_t& code, obj::Object* input) {

        return execute<SORTED>(code.commands, code.rt, input);
    }

    static obj::Object* make(const Type& t) {
        obj::Object* ret = obj::make<SORTED>(t);

        if (!ret)
            throw std::runtime_error("Cannot make() a " + Type::print(t));

        return ret;
    }
};

} // namespace tab

#endif
