# Embedding tab #

A short exposition on how to embded the `tab` language into your own programs.

Here is a complete example program that evaluates a `tab` expression of type `Int, Int -> Real` ten thousand times:

    :::c++
    #include "tab.h"
    
    int main(int argc, char** argv) {
    
        using T = tab::Type;
        using API = tab::API<false>;
    
        size_t seed = std::stoul(argv[2]);
        API::init(seed);
    
        static T in_type(T::TUP, { T::INT, T::INT });
        static T out_type(T::REAL);
    
        typename API::compiled_t code;
    
        std::string program(argv[1]);
        API::compile(program.begin(), program.end(), in_type, code);
    
        if (code.result != out_type)
            throw std::runtime_error("Error: function must return Real.");
    
        static tab::obj::Tuple* input = API::make(in_type);
    
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {

                namespace to = tab::obj;
    
                to::Tuple& t = to::get<to::Tuple>(input);
                to::get<to::Int>(t.v[0]).v = i;
                to::get<to::Int>(t.v[1]).v = j;
    
                tab::obj::Object* output = API::run(code, input);
    
                std::cout << to::get<to::Real>(output).v << std::endl;
            }
        }
    
        return 0;
    }

(Please compile with maximum optimization, `-O3`, or your program will run slow.)

* An expression is compiled once and can be evaluated many times with different inputs.
* An expression must have only one input type and only one output type.
* Compile expressions with `API::compile` and evaluate them with `API::run`.
* The template argument of `API` is whether or not to use sorted (binary tree) or unsorted (hash table) maps.
* Values are represented by pointers to objects derived from `obj::Object`.
* Values, as a general rule, are allocated once and never freed.

For getting data into and out of values, you'll need to delve into the implementation specifics:

* Atomic values are represented by `obj::Atom<T>`, a.k.a. `obj::Int`, `obj::UInt`, `obj::Real` and `obj::String`.
* Arrays of atomic values are represented by `obj::ArrayAtom<T>`.
* Arrays of any other type of value are represented by `obj::ArrayObject`.
* Tuples are represented by `obj::Tuple`.
* Maps are represented by `obj::MapObject<bool>`, where the argument, again, is whether or not a sorted map should be used.
* Sequences are objects derived from `obj::SeqBase`.

Sequences must have a method `obj::Object* next()` which returns the next value in a sequence or `nullptr` to flag an end-of-sequence.

All other values have a member variable `v` that holds the corresponding C++ value.

You can construct a default value from a type by calling `API::make`. (Doesn't work with sequences.)

Use `obj::get<T>` to cast an `obj::Object*` to a concrete value. (No run-time type checking is done, so take care.)

