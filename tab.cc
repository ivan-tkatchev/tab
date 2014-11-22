
#include <memory>
#include <stdexcept>
#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <initializer_list>

#include <iostream>

#include "axe.h"


typedef long Int;
typedef unsigned long UInt;
typedef double Real;

struct String {
    size_t ix;

    bool operator==(String b) const { return ix == b.ix; }
};

namespace std {

template <> struct hash<String> {
    size_t operator()(String x) const { return hash<size_t>()(x.ix); }
};

}

struct Strings {

    std::unordered_map<std::string,size_t> s2i;
    std::unordered_map<size_t,std::string> i2s;
    
    String add(const std::string& s) {
        auto i = s2i.find(s);

        if (i != s2i.end())
            return String{i->second};

        size_t n = s2i.size() + 1;
        s2i.insert(std::make_pair(s, n));
        i2s.insert(std::make_pair(n, s));
        return String{n};
    }

    const std::string& get(String s) {
        auto i = i2s.find(s.ix);

        if (i == i2s.end())
            throw std::runtime_error("Sanity error: uninterned string.");

        return i->second;
    }
};

Strings& strings() {
    static Strings ret;
    return ret;
}


struct Atom {

    enum {
        INT = 0,
        UINT = 1,
        REAL = 2,
        STRING = 3
    } which;

    union {
        Int inte;
        UInt uint;
        Real real;
        String str;
    };

    Atom(Int i = 0) : which(INT), inte(i) {}
    Atom(UInt i) : which(UINT), uint(i)  {}
    Atom(Real i) : which(REAL), real(i)  {}
    Atom(String i) : which(STRING), str(i) {}

    void copy(const Atom& a) {

        switch (a.which) {
        case INT:
            inte = a.inte;
            break;
        case UINT:
            uint = a.uint;
            break;
        case REAL:
            real = a.real;
            break;
        case STRING:
            str = a.str;
            break;
        };

        which = a.which;
    }
    
    Atom(const Atom& a) {        
        copy(a);
    }

    ~Atom() {}

    Atom& operator=(const Atom& a) {
        copy(a);
        return *this;
    }

    bool is_string() const { return (which == STRING); }
    
    std::string print() const {
        switch (which) {
        case INT:
            return std::to_string(inte);
        case UINT:
            return std::to_string(uint);
        case REAL:
            return std::to_string(real);
        case STRING:
            return strings().get(str);
        }
        return ":~(";
    }
};
   
struct Type {

    enum types_t {
        ATOM,
        ARR,
        MAP,
        NONE
    };
    
    enum atom_types_t {
        INT,
        UINT,
        REAL,
        STRING
    };

    types_t type;
    atom_types_t atom;
    std::shared_ptr< std::vector<Type> > tuple;
    
    Type(types_t t = NONE) : type(t) {}

    Type(atom_types_t a) : type(ATOM), atom(a) {}
        
    Type(const Atom& a) : type(ATOM) {
        switch (a.which) {
        case INT: atom = INT; break;
        case UINT: atom = UINT; break;
        case REAL: atom = REAL; break;
        case STRING: atom = STRING; break;
        }
    }

    Type(types_t t, const std::initializer_list<Type>& tup) :
        type(t),
        tuple(std::make_shared< std::vector<Type> >(tup)) {}

    
    bool operator!=(const Type& t) const {
        if (t.type != type) return true;

        if (type == ATOM) {

            if (atom != t.atom)
                return true;

            return false;
        }
            
        if (tuple && t.tuple) {

            if (tuple->size() != t.tuple->size())
                return true;

            for (size_t i = 0; i < tuple->size(); ++i) {
                if ((*tuple)[i] != (*t.tuple)[i])
                    return true;
            }

            return false;

        } else if (!tuple && !t.tuple) {
            return false;

        } else {
            return true;
        }
    }

    bool operator==(const Type& t) const {
        return !(t != *this);
    }
    
    static std::string print(const Type& t) {
        std::string ret;

        switch (t.type) {
        case NONE: ret += "NONE"; break;
        case ATOM:
            switch (t.atom) {
            case INT: ret += "INT"; break;
            case UINT: ret += "UINT"; break;
            case REAL: ret += "REAL"; break;
            case STRING: ret += "STRING"; break;
            }
            break;
        case ARR:  ret += "ARRAY"; break;
        case MAP:  ret += "MAP"; break;
        }

        if (t.tuple) {
            ret += "(";

            for (const Type& tt : *(t.tuple)) {
                ret += " ";
                ret += print(tt);
            }

            ret += " )";
        }

        return ret;
    }

    Type& push(const Type& t) {

        if (!tuple) {
            tuple = std::make_shared< std::vector<Type> >();
        }

        tuple->push_back(t);
        return tuple->back();
    }
};

struct Command {

    enum cmd_t {
        VAL,
        VAW,
        VAR,
        NOT,
        NEG,
        EXP,
        MUL,
        DIV,
        MOD,
        ADD,
        SUB,
        AND,
        OR,
        XOR,
        IDX,
        REGEX,

        ARR,
        MAP,
        FUN
    };

    cmd_t cmd;

    Atom arg;

    typedef std::shared_ptr< std::vector<Command> > closure_t;

    std::vector<closure_t> closure;

    Type type;
    void* object;
    void* function;
    
    Command(cmd_t c = VAL) : cmd(c), object(nullptr), function(nullptr) {}

    template <typename T>
    Command(cmd_t c, const T& t) : cmd(c), arg(t), object(nullptr), function(nullptr) {}

    static std::string print(cmd_t c) {
        switch (c) {
        case VAL: return "VAL";
        case VAW: return "VAW";
        case VAR: return "VAR";
        case NOT: return "NOT";
        case NEG: return "NEG";
        case EXP: return "EXP";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case MOD: return "MOD";
        case ADD: return "ADD";
        case SUB: return "SUB";
        case AND: return "AND";
        case  OR: return "OR";
        case XOR: return "XOR";
        case IDX: return "IDX";
        case REGEX: return "REGEX";
        case ARR: return "ARR";
        case MAP: return "MAP";
        case FUN: return "FUN";
        }
        return ":~(";
    }
};

union Raw {
    Int inte;
    UInt uint;
    Real real;
    void* ptr;

    Raw(Int i = 0) : inte(i) {}
    Raw(UInt i) : uint(i) {}
    Raw(Real i) : real(i) {}
    Raw(void* i) : ptr(i) {}
    
    template <typename T>
    T& get() const {
        return *((T*)ptr);
    }
};


namespace funcs {

void print_int(const std::vector<Raw>& in, Raw& out) {
    std::cout << in[0].inte << std::endl;
}

void cut(const std::vector<Raw>& in, Raw& out) {

    const std::string& str = in[0].get<std::string>();
    const std::string& del = in[1].get<std::string>();

    size_t N = str.size();
    size_t M = del.size();

    size_t prev = 0;

    if (out.ptr == nullptr) {
        out.ptr = new std::vector<std::string>();
    }

    std::vector<std::string>& v = out.get< std::vector<std::string> >();

    v.clear();
    v.emplace_back("");
    
    for (size_t i = 0; i < N; ++i) {

        bool matched = true;

        for (size_t j = 0; j < M; ++j) {

            if (i+j < N && str[i+j] == del[j])
                continue;
            
            matched = false;
            break;
        }

        if (matched) {
            v.emplace_back(str.begin() + prev, str.begin() + i);
            i += M;
            prev = i;
        } else {
            v.back() += str[i];
        }
    }
}

}

namespace std {

template <>
struct hash<Type> {
    size_t operator()(const Type& t) const {

        size_t r = hash<size_t>()(t.type);

        if (t.type == Type::ATOM) {
            r += hash<size_t>()(t.atom);

        } else if (t.tuple) {
        
            for (auto i : (*t.tuple)) {
                r += (*this)(i);
            }
        }

        return r;
    }
};

template <>
struct hash< std::pair<String, std::vector<Type> > > {
    size_t operator()(const std::pair<String, std::vector<Type> >& x) const {

        size_t r = hash<size_t>()(x.first.ix);

        for (const Type& t : x.second) {
            r += hash<Type>()(t);
        }

        return r;
    }
};

}

struct Functions {

    typedef void (*func_t)(const std::vector<Raw>&, Raw&);

    typedef std::pair< String, std::vector<Type> > key_t;

    std::unordered_map<key_t, func_t> funcs;
    std::unordered_map<String, Type> types;

    Functions() {
        
        add("cut",
            { Type(Type::STRING), Type(Type::STRING) },
            Type(Type::ARR, { Type::STRING }),
            funcs::cut);

        add("print",
            { Type(Type::INT) },
            Type(),
            funcs::print_int);
    }

    void add(const std::string& name, const std::initializer_list<Type>& args, const Type& out, func_t f) {
        
        String n = strings().add(name);
        funcs.insert(funcs.end(), std::make_pair(key_t(n, std::vector<Type>(args)), f));
        types.insert(types.end(), std::make_pair(n, out));
    }

    func_t get_func(const String& name, const std::vector<Type>& args) const {

        auto i = funcs.find(key_t(name, args));

        if (i == funcs.end()) {

            std::string tmp;
            for (auto z : args) {
                tmp += Type::print(z);
                tmp += ',';
            }

            if (tmp.size() > 1) {
                tmp.pop_back();
            }

            throw std::runtime_error("Invalid function call: " + strings().get(name) + "(" + tmp + ")");
        }

        return i->second;
    }

    Type get_type(const String& name) const {

        auto i = types.find(name);
    
        if (i == types.end())
            throw std::runtime_error("Unknown function: '" + strings().get(name) + "'");

        return i->second;
    }
}; 

const Functions& functions() {
    static Functions ret;
    return ret;
}



bool check_integer(const Type& t) {
    return (t.type == Type::ATOM && (t.atom == Type::INT || t.atom == Type::UINT));
}

bool check_real(const Type& t) {
    return (t.type == Type::ATOM && t.atom == Type::REAL);
}

bool check_numeric(const Type& t) {
    return (t.type == Type::ATOM && (t.atom == Type::INT || t.atom == Type::UINT || t.atom == Type::REAL));
}

bool check_string(const Type& t) {
    return (t.type == Type::ATOM && t.atom == Type::STRING);
}

void handle_real_operator(std::vector<Type>& stack, const std::string& name) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    stack.emplace_back(Type::REAL);
}

void handle_int_operator(std::vector<Type>& stack, const std::string& name) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_integer(t1) || !check_integer(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-integer value.");

    auto a1 = t1.atom;
    auto a2 = t2.atom;

    if (a1 == Type::UINT && a2 == Type::UINT) {
        stack.emplace_back(Type::UINT);

    } else {
        stack.emplace_back(Type::INT);
    }
}

void handle_poly_operator(std::vector<Type>& stack, const std::string& name, bool always_int = false) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    auto a1 = t1.atom;
    auto a2 = t2.atom;

    if (a1 == Type::REAL || a2 == Type::REAL) {
        stack.emplace_back(Type::REAL);

    } else if (!always_int && a1 == Type::UINT && a2 == Type::UINT) {
        stack.emplace_back(Type::UINT);

    } else {
        stack.emplace_back(Type::INT);
    }
}


struct TypeResult {

    std::vector<Type> stack;
    std::unordered_map<String, Type> vars;
};


Type stack_to_type(const TypeResult& typer, const std::string& name) {

    if (typer.stack.size() == 0)
        throw std::runtime_error("Empty sequences are not allowed.");
    
    Type ret;

    if (typer.stack.size() == 1) {
        ret = typer.stack[0];
        
    } else {

        ret.type = Type::ARR;
        
        for (const auto& c : typer.stack) {
            ret.push(c);
        }
    }
    
    return ret;
}

void infer_types(std::vector<Command>& commands, const Type& toplevel, TypeResult& typer);

std::vector<Type> infer_func_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() != 1)
        throw std::runtime_error("Sanity error, funcall is not a closure.");

    TypeResult typer;
    typer.vars = _tr.vars;

    auto& cto = *(c.closure.at(0));
    infer_types(cto, toplevel, typer);

    return typer.stack;
}

Type infer_arr_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() < 1 || c.closure.size() > 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    TypeResult typer;
    typer.vars = _tr.vars;
    
    if (c.closure.size() == 2) {

        auto& cfrom = *(c.closure.at(1));

        infer_types(cfrom, toplevel, typer);

        toplevel = stack_to_type(typer, name);
    }
    
    auto& cto = *(c.closure.at(0));
    infer_types(cto, toplevel, typer);

    return stack_to_type(typer, name);
}

Type infer_map_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() < 2 || c.closure.size() > 3)
        throw std::runtime_error("Sanity error, generator is not a map closure.");

    TypeResult typer;
    typer.vars = _tr.vars;
    
    if (c.closure.size() == 3) {

        auto& cfrom = *(c.closure.at(2));
        infer_types(cfrom, toplevel, typer);

        toplevel = stack_to_type(typer, name);
    }
    
    auto& cto_k = *(c.closure.at(0));
    infer_types(cto_k, toplevel, typer);
    Type out1 = stack_to_type(typer, name);

    auto& cto_v = *(c.closure.at(1));
    infer_types(cto_v, toplevel, typer);    
    Type out2 = stack_to_type(typer, name);

    Type ret(Type::MAP);
    ret.push(out1);
    ret.push(out2);
    
    return ret;
}

Type value_type(const Type& t) {

    if (!t.tuple || t.tuple->empty())
        throw std::runtime_error("Indexing an atom.");

    Type ret = t;

    if (t.type == Type::ARR) {

        if (t.tuple->size() == 1) {
            return (*t.tuple)[0];

        } else {
            return t;
        }

    } else if (t.type == Type::MAP) {

        if (t.tuple->size() != 2)
            throw std::runtime_error("Sanity error, degenerate map");

        return (*t.tuple)[1];
    }
        
    throw std::runtime_error("Sanity error, indexing something that's not array or map");
}

Type mapped_type(const Type& t) {

    if (t.type != Type::MAP || !t.tuple || t.tuple->size() != 2)
        throw std::runtime_error("Sanity error, degenerate map");

    return (*t.tuple)[0];
}

void infer_types(std::vector<Command>& commands, const Type& toplevel, TypeResult& typer) {

    auto& stack = typer.stack;
    auto& vars = typer.vars;

    stack.clear();
    vars[strings().add("$")] = toplevel;
    
    for (auto& c : commands) {

        switch (c.cmd) {
        case Command::VAL:
            stack.emplace_back(c.arg);
            break;

        case Command::VAW:
            vars[c.arg.str] = Type(stack.back());
            stack.pop_back();
            break;
            
        case Command::VAR:
        {
            auto i = vars.find(c.arg.str);

            if (i == vars.end())
                throw std::runtime_error("Use of undefined variable: " + strings().get(c.arg.str));

            stack.emplace_back(i->second);
            break;
        }
        
        case Command::NOT:
            stack.pop_back();
            stack.emplace_back(Type::INT);
            break;

        case Command::NEG:
        {
            Type t = stack.back();

            if (!check_integer(t)) 
                throw std::runtime_error("Use of '~' numeric operator on something other "
                                         "than integer or unsigned integer.");

            break;
        }
            
        case Command::EXP:
            handle_real_operator(stack, "**");
            break;

        case Command::MUL:
            handle_poly_operator(stack, "*");
            break;

        case Command::DIV:
            handle_poly_operator(stack, "/");
            break;

        case Command::MOD:
            handle_int_operator(stack, "%");
            break;

        case Command::ADD:
            handle_poly_operator(stack, "+");
            break;

        case Command::SUB:
            handle_poly_operator(stack, "-", true);
            break;

        case Command::AND:
            handle_int_operator(stack, "&");
            break;

        case Command::OR:
            handle_int_operator(stack, "|");
            break;

        case Command::XOR:
            handle_int_operator(stack, "^");
            break;

        case Command::REGEX:
        {
            Type t = stack.back();
            stack.pop_back();
            
            if (!check_string(t)) 
                throw std::runtime_error("Use of '~' regex operator on something other than string.");

            stack.emplace_back(Type::ARR);
            stack.back().push(Type(Type::STRING));
            break;
        }

        case Command::IDX:
        {
            Type ti = stack.back();
            stack.pop_back();
            Type tv = stack.back();
            stack.pop_back();

            switch (tv.type) {

            case Type::ARR:

                if (!check_numeric(ti))
                    throw std::runtime_error("Arrays must be accessed with numeric index.");

                stack.emplace_back(value_type(tv));
                break;

            case Type::MAP:

                stack.emplace_back(mapped_type(tv));

                if (ti != value_type(tv))
                    throw std::runtime_error("Invalid key type when accessing map.");
                break;
                    
            default:
                throw std::runtime_error("Cannot index a scalar value.");
            }

            break;
        }
        
        case Command::ARR:
        {
            Type t = infer_arr_generator(c, toplevel, typer, "array");
            stack.emplace_back(t);
            break;
        }

        case Command::MAP:
        {
            Type t = infer_map_generator(c, toplevel, typer, "map");
            stack.emplace_back(t);
            break;
        }

        case Command::FUN:
        {
            Type t = functions().get_type(c.arg.str);
            stack.emplace_back(t);

            std::vector<Type> args = infer_func_generator(c, toplevel, typer, "function call");
            c.function = (void*)functions().get_func(c.arg.str, args);
            break;
        }
        }

        if (c.cmd != Command::VAW) {
            c.type = stack.back();
        }
    }
}

struct Stack {

    std::vector<Command> stack;

    void push(Command::cmd_t c) { stack.emplace_back(c); }

    template <typename T>
    void push(Command::cmd_t c, const T& t) { stack.emplace_back(c, t); }

    std::vector< std::pair<size_t,String> > _mark;
    std::vector< String> names;
    
    void mark() {
        _mark.emplace_back(stack.size(), String{0});
    }

    void mark(String n) {
        _mark.emplace_back(stack.size(), n);
    }

    void unmark() {
        _mark.pop_back();
    }
    
    void close(Command::cmd_t cmd) {

        auto m = _mark.back();
        _mark.pop_back();

        auto c = std::make_shared< std::vector<Command> >(stack.begin() + m.first, stack.end());
        stack.erase(stack.begin() + m.first, stack.end());

        if (m.second.ix == 0) {
            stack.emplace_back(cmd);
        } else {
            stack.emplace_back(cmd, m.second);
        }
        
        stack.back().closure.resize(1);
        stack.back().closure[0].swap(c);
    }

    void close() {

        auto m = _mark.back();
        _mark.pop_back();

        auto c = std::make_shared< std::vector<Command> >(stack.begin() + m.first, stack.end());
        stack.erase(stack.begin() + m.first, stack.end());

        stack.back().closure.resize(stack.back().closure.size() + 1);
        stack.back().closure.back().swap(c);
    }

    static void print(const std::vector<Command>& c, size_t level) {

        for (const auto& i : c) {
            std::cout << " " << std::string(level*2, ' ')
                      << Command::print(i.cmd) << " " << i.arg.which << ": " << i.arg.print()
                      << " // " << Type::print(i.type) << std::endl;

            for (const auto& ii : i.closure) {
                std::cout << " " << std::string(level*2, ' ') << "=" << std::endl;
                print(*ii, level + 1);
            }
        }
    }        
    
    void print() const {
        print(stack, 0);
    }
};

template <typename I>
String make_string(I beg, I end) {
    return strings().add(std::string(beg, end));
}

template <typename I>
void parse(I beg, I end, TypeResult& typer, std::vector<Command>& commands) {

    Stack stack;
    std::string str_buff;
    
    axe::r_rule<I> x_expr;
    axe::r_rule<I> x_expr_atom;
    
    auto x_ws = *axe::r_any(" \t\n");

    auto y_int = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, std::stol(std::string(b, e))); });
    
    auto x_int = (~axe::r_lit('-') & +axe::r_num())
        >> y_int;

    auto y_uint = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, std::stoul(std::string(b, e))); });
    
    auto x_uint = (+axe::r_num() & axe::r_lit('u'))
        >> y_uint;

    auto y_float = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, std::stod(std::string(b, e))); });
    
    auto x_floatlit = ~axe::r_any("-+") & +axe::r_num() & axe::r_lit('.') & axe::r_many(axe::r_num(),0);
    auto x_floatexp = ~axe::r_any("-+") & +axe::r_num() & axe::r_any("eE") & ~axe::r_any("-+") & +axe::r_num();
    auto x_float = (x_floatlit | x_floatexp)
        >> y_float;

    auto y_string_start = axe::e_ref([&](I b, I e) { str_buff.clear(); });
    auto y_string_end = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, strings().add(str_buff)); });
    auto y_quotedchar = axe::e_ref([&](I b, I e) {

            std::string z(b, e);

            if (z == "t") {
                str_buff += '\t';
            } else if (z == "n") {
                str_buff += '\n';
            } else if (z == "e") {
                str_buff += '\e';
            } else {
                str_buff += z;
            }
        });
    auto y_char = axe::e_ref([&](I b, I e) { str_buff += *b; });
    
    auto x_quotedchar = axe::r_lit("\\") > (axe::r_any() >> y_quotedchar);
    auto x_char1 = x_quotedchar | (axe::r_any() - axe::r_lit('\\') - axe::r_lit('"')) >> y_char;
    auto x_char2 = x_quotedchar | (axe::r_any() - axe::r_lit('\\') - axe::r_lit('\'')) >> y_char;
    auto x_string =
        (axe::r_lit('"')  >> y_string_start & axe::r_many(x_char1,0) & axe::r_lit('"')  >> y_string_end) |
        (axe::r_lit('\'') >> y_string_start & axe::r_many(x_char2,0) & axe::r_lit('\'') >> y_string_end);

    auto x_literal = x_float | x_uint | x_int | x_string;

    auto x_var = axe::r_lit('$') | (axe::r_alpha() & axe::r_many(axe::r_alnum() | axe::r_lit('_'),0));

    auto y_mark = axe::e_ref([&](I b, I e) { stack.mark(); });
    auto y_mark_name = axe::e_ref([&](I b, I e) { stack.mark(make_string(b, e)); });
    auto y_unmark_name = axe::e_ref([&](I b, I e) { stack.unmark(); });
    auto y_close_arg = axe::e_ref([&](I b, I e) { stack.close(); });
    auto y_close_arr = axe::e_ref([&](I b, I e) { stack.close(Command::ARR); });
    auto y_close_map = axe::e_ref([&](I b, I e) { stack.close(Command::MAP); });
    auto y_close_fun = axe::e_ref([&](I b, I e) { stack.close(Command::FUN); });

    auto y_true = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, (Int)1); });
    
    auto x_from = ~((axe::r_lit(':') >> y_mark) & (x_expr >> y_close_arg));

    auto x_array =
        (axe::r_lit('[') >> y_mark) & (x_expr >> y_close_arr) & x_from & axe::r_lit(']');
    
    auto x_map =
        (axe::r_lit('{')  >> y_mark) & (x_expr >> y_close_map) &
        (((axe::r_lit("->") >> y_mark) & (x_expr >> y_close_arg)) |
         (axe::r_empty() >> y_mark >> y_true >> y_close_arg)) &
        x_from & axe::r_lit('}');
    
    auto x_funcall =
        (x_var >> y_mark_name) &
        x_ws &
        (axe::r_lit('(') | r_fail(y_unmark_name)) &
        ~x_expr &
        axe::r_lit(')') >> y_close_fun;

    auto y_var_read = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, make_string(b, e)); });
    
    auto x_var_read = x_var >> y_var_read;

    auto x_expr_bottom =
        x_ws &
        (x_literal | x_funcall | x_var_read | x_array | x_map |
         (axe::r_lit('(') & x_expr_atom & axe::r_lit(')'))) &
        x_ws;

    auto y_expr_idx = axe::e_ref([&](I b, I e) { stack.push(Command::IDX); });
    
    auto x_expr_idx =
        x_expr_bottom & ~(x_array >> y_expr_idx) & x_ws;

    auto y_expr_not = axe::e_ref([&](I b, I e) { stack.push(Command::NOT); });
    auto y_expr_neg = axe::e_ref([&](I b, I e) { stack.push(Command::NEG); });
    
    auto x_expr_neg =
        (axe::r_any("!") & x_expr_atom >> y_expr_not) |
        (axe::r_any("~") & x_expr_atom >> y_expr_neg) |
        x_expr_idx;

    auto y_expr_exp = axe::e_ref([&](I b, I e) { stack.push(Command::EXP); });
    
    auto x_expr_exp =
        x_expr_neg & ~(axe::r_lit("**") & x_expr_atom >> y_expr_exp);

    auto y_expr_mul = axe::e_ref([&](I b, I e) { stack.push(Command::MUL); });
    auto y_expr_div = axe::e_ref([&](I b, I e) { stack.push(Command::DIV); });
    auto y_expr_mod = axe::e_ref([&](I b, I e) { stack.push(Command::MOD); });
    
    auto x_expr_mul =
        x_expr_exp & ~((axe::r_lit('*') & x_expr_atom) >> y_expr_mul |
                       (axe::r_lit('/') & x_expr_atom) >> y_expr_div |
                       (axe::r_lit('%') & x_expr_atom) >> y_expr_mod);

    auto y_expr_add = axe::e_ref([&](I b, I e) { stack.push(Command::ADD); });
    auto y_expr_sub = axe::e_ref([&](I b, I e) { stack.push(Command::SUB); });
    
    auto x_expr_add =
        x_expr_mul & ~((axe::r_lit('+') & x_expr_atom) >> y_expr_add |
                       (axe::r_lit('-') & x_expr_atom) >> y_expr_sub);

    auto y_expr_and = axe::e_ref([&](I b, I e) { stack.push(Command::AND); });
    auto y_expr_or  = axe::e_ref([&](I b, I e) { stack.push(Command::OR); });
    auto y_expr_xor = axe::e_ref([&](I b, I e) { stack.push(Command::XOR); });

    auto x_expr_bit =
        x_expr_add & ~((axe::r_lit('&') & x_expr_atom) >> y_expr_and |
                       (axe::r_lit('|') & x_expr_atom) >> y_expr_or |
                       (axe::r_lit('^') & x_expr_atom) >> y_expr_xor);

    auto y_expr_regex = axe::e_ref([&](I b, I e) { stack.stack.back().cmd = Command::REGEX; });

    auto x_expr_regex =
        x_expr_bit & ~(axe::r_lit("~") & x_ws & x_string >> y_expr_regex);

    x_expr_atom = x_expr_regex;

    auto y_expr_assign_var = axe::e_ref([&](I b, I e) { stack.names.emplace_back(make_string(b, e)); });
    auto y_expr_assign = axe::e_ref([&](I b, I e) { stack.push(Command::VAW, stack.names.back());
                                                    stack.names.pop_back(); });

    auto y_no_assign = axe::e_ref([&](I b, I e) { stack.names.pop_back(); });
    
    auto x_expr_assign = 
        (x_ws &
         (x_var >> y_expr_assign_var) &
         x_ws &
         (axe::r_lit('=') | r_fail(y_no_assign)) &
         x_ws &
         (x_expr_atom >> y_expr_assign)) | 
        x_expr_atom;

    auto x_expr_seq = x_expr_assign & *(axe::r_lit(',') & x_expr_assign);

    x_expr = x_expr_seq;

    auto x_main = x_expr & axe::r_end();

    auto x_go = x_main |
        axe::r_fail([](I b, I e) {
                throw std::runtime_error("Syntax error, unparsed input: \"" + std::string(b, e) + "\"");
            });
    
    x_go(beg, end);


    // Wrap the result in an implicit 'print(...)' function.

    Stack trustack;
    /*
    trustack.push(Command::FUN, strings().add("print"));
    Command& print = trustack.stack.back();
    print.closure.clear();
    print.closure.emplace_back(new std::vector<Command>);
    print.closure[0]->swap(stack.stack);
    */
    trustack = stack;
    
    infer_types(trustack.stack, Type(Type::STRING), typer);

    trustack.print();

    commands.swap(trustack.stack);
}

struct Runtime {
    std::unordered_map<String,Raw> vars;
    std::vector<Raw> stack;

    void set_toplevel(const std::string& s) {
        vars[strings().add("$")] = Raw((void*)(&s));
    }
};

void execute(std::vector<Command>& commands, Runtime& r) {
    
    for (auto& c : commands) {
        switch (c.cmd) {

        case Command::FUN:
        {
            Runtime rfun;
            rfun.vars = r.vars;
            execute(*(c.closure[0]), rfun);

            r.stack.emplace_back();
            Raw& out = r.stack.back();
            out.ptr = c.object;
            ((Functions::func_t)c.function)(rfun.stack, out);
            c.object = out.ptr;
            break;
        }
        case Command::VAR:
        {
            r.stack.emplace_back(r.vars[c.arg.str]);
            break;
        }
        case Command::VAL:
        {
            switch (c.arg.which) {
            case Atom::STRING:
                r.stack.emplace_back((void*)(&(strings().get(c.arg.str))));
                break;
            case Atom::INT:
                r.stack.emplace_back(c.arg.inte);
                break;
            case Atom::UINT:
                r.stack.emplace_back(c.arg.uint);
                break;
            case Atom::REAL:
                r.stack.emplace_back(c.arg.real);
                break;
            }
            break;
        }
        default:
            break;
        }
    }        
}

        
int main(int argc, char** argv) {

    try {

        if (argc != 2 && argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <expression> [arg]" << std::endl;
            return 1;
        }

        std::string program(argv[1]);
        std::string toplevel;

        if (argc == 3) {
            toplevel.assign(argv[2]);
        }
        
        TypeResult typer;
        std::vector<Command> commands;

        parse(program.begin(), program.end(), typer, commands);

        Runtime res;
        res.set_toplevel(toplevel);
        execute(commands, res);
        
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
