
#include <memory>
#include <stdexcept>
#include <functional>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <initializer_list>
#include <utility>

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
        TUP,
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
        case TUP: ret += "TUPLE"; break;
        case ARR: ret += "ARRAY"; break;
        case MAP: ret += "MAP"; break;
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

namespace obj {

struct Object;

}

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

    struct Closure {
        std::vector<Command> code;
        Type type;
        obj::Object* object;

        Closure() : object(nullptr) {}
    };
    
    std::vector< std::shared_ptr<Closure> > closure;

    Type type;
    obj::Object* object;
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
struct hash< std::pair<String, Type> > {
    size_t operator()(const std::pair<String, Type>& x) const {

        return hash<size_t>()(x.first.ix) + hash<Type>()(x.second);
    }
};

}

struct Functions {

    typedef void (*func_t)(const obj::Object*, obj::Object*&);

    typedef std::pair< String, Type > key_t;
    typedef std::pair< func_t, Type > val_t;
    
    std::unordered_map<key_t, val_t> funcs;

    Functions() {}

    void add(const std::string& name, const Type& args, const Type& out, func_t f) {
        
        String n = strings().add(name);
        funcs.insert(funcs.end(), std::make_pair(key_t(n, args), val_t(f, out)));
    }

    val_t get(const String& name, const Type& args) const {
            
        auto i = funcs.find(key_t(name, args));

        if (i == funcs.end()) {

            throw std::runtime_error("Invalid function call: " + strings().get(name) + " " + Type::print(args));
        }

        return i->second;
    }
}; 

Functions& functions_init() {
    static Functions ret;
    return ret;
}

const Functions& functions() {
    return functions_init();
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

        ret.type = Type::TUP;
        
        for (const auto& c : typer.stack) {
            ret.push(c);
        }
    }
    
    return ret;
}

void infer_types(std::vector<Command>& commands, const Type& toplevel, TypeResult& typer);

Type infer_tup_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() != 1)
        throw std::runtime_error("Sanity error, " + name + " is not a closure.");

    TypeResult typer;
    typer.vars = _tr.vars;

    auto& cto = *(c.closure.at(0));
    infer_types(cto.code, toplevel, typer);
    cto.type = stack_to_type(typer, name);
    
    return cto.type;
}

Type infer_arr_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() != 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    TypeResult typer;
    typer.vars = _tr.vars;
    
    auto& cfrom = *(c.closure.at(1));

    infer_types(cfrom.code, toplevel, typer);
    cfrom.type = stack_to_type(typer, name);

    toplevel = cfrom.type;
    
    auto& cto = *(c.closure.at(0));
    infer_types(cto.code, toplevel, typer);
    cto.type = stack_to_type(typer, name);

    Type ret(Type::ARR);
    ret.push(cto.type);

    return ret;
}

Type infer_map_generator(Command& c, Type toplevel, const TypeResult& _tr, const std::string& name) {

    if (c.closure.size() != 3)
        throw std::runtime_error("Sanity error, generator is not a map closure.");

    TypeResult typer;
    typer.vars = _tr.vars;
    
    auto& cfrom = *(c.closure.at(2));
    infer_types(cfrom.code, toplevel, typer);
    cfrom.type = stack_to_type(typer, name);

    toplevel = cfrom.type;
    
    auto& cto_k = *(c.closure.at(0));
    infer_types(cto_k.code, toplevel, typer);
    cto_k.type = stack_to_type(typer, name);

    auto& cto_v = *(c.closure.at(1));
    infer_types(cto_v.code, toplevel, typer);
    cto_v.type = stack_to_type(typer, name);

    Type ret(Type::MAP);
    ret.push(cto_k.type);
    ret.push(cto_v.type);

    return ret;
}

Type value_type(const Type& t) {

    if (!t.tuple || t.tuple->empty())
        throw std::runtime_error("Indexing an atom.");

    Type ret = t;

    if (t.type == Type::TUP) {
        throw std::runtime_error("Cannot index tuples at runtime");
        
    } else if (t.type == Type::ARR) {

        if (t.tuple->size() != 1)
            throw std::runtime_error("Sanity error, degenerate array");
        
        return (*t.tuple)[0];

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
            Type tv = stack.back();
            stack.pop_back();

            Type ti = infer_tup_generator(c, toplevel, typer, "structure index");

            switch (tv.type) {

            case Type::TUP:
            {
                bool ok = false;
                auto& cl = *(c.closure.at(0));
                
                if (cl.code.size() == 1) {

                    const auto& v = cl.code[0];

                    if (v.cmd == Command::VAL && (v.arg.which == Atom::INT || v.arg.which == Atom::UINT)) {

                        UInt i = v.arg.uint;

                        if (tv.tuple && i < tv.tuple->size()) {

                            stack.emplace_back(tv.tuple->at(i));
                            ok = true;
                        }
                    }
                }

                if (!ok)
                    throw std::runtime_error("Indexing tuples is only possible with integer literals.");

                break;
            }
            
            case Type::ARR:
                
                if (!check_numeric(ti))
                    throw std::runtime_error("Arrays must be accessed with numeric index.");

                stack.emplace_back(value_type(tv));
                break;

            case Type::MAP:
                
                stack.emplace_back(value_type(tv));

                if (ti != mapped_type(tv))
                    throw std::runtime_error("Invalid key type when accessing map: key is " +
                                             Type::print(mapped_type(tv)) + ", acessing with " +
                                             Type::print(ti));
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
            Type args = infer_tup_generator(c, toplevel, typer, "function call");
            auto tmp = functions().get(c.arg.str, args);
            c.function = (void*)tmp.first;
            stack.emplace_back(tmp.second);
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

        auto c = std::make_shared<Command::Closure>();
        c->code.assign(stack.begin() + m.first, stack.end());
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

        auto c = std::make_shared<Command::Closure>();
        c->code.assign(stack.begin() + m.first, stack.end());
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

                std::cout << " " << std::string(level*2, ' ') << "= " << Type::print(ii->type) << std::endl;

                print(ii->code, level + 1);
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

    auto y_default_from = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, strings().add("$")); });
    
    auto x_from =
        (((axe::r_lit(':') >> y_mark) & (x_expr >> y_close_arg)) |
         (axe::r_empty() >> y_mark >> y_default_from >> y_close_arg));

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

    auto y_close_idx = axe::e_ref([&](I b, I e) { stack.close(Command::IDX); });
    auto x_index = (axe::r_lit('[') >> y_mark) & x_expr & axe::r_lit(']') >> y_close_idx;
    
    auto x_expr_idx =
        x_expr_bottom & ~(x_index) & x_ws;

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

    
    infer_types(stack.stack, Type(Type::STRING), typer);

    stack.print();

    commands.swap(stack.stack);
}


/** ** **/


namespace obj {

struct Object {

    virtual ~Object() {}
    
    virtual void index(const Type& keytype, Object* key, Object*& out) const {
        throw std::runtime_error("Sanity error, indexing a non-indexable Object");
    }

    virtual size_t hash() const {
        throw std::runtime_error("Object hash not implemented");
    }

    virtual bool eq(Object*) const {
        throw std::runtime_error("Object equality not implemented");
    }

    virtual void print() const {}
    
    virtual void set(const std::vector<Object*>&) {
        throw std::runtime_error("Object assignment not implemented");
    }

    virtual Object* clone() const {
        throw std::runtime_error("Object cloning not implemented");
    }

    virtual void map(Object*, Object*) {
        throw std::runtime_error("Object map construction not implemented");
    }
};

template <typename T>
T& get(const Object* o) {
    return *((T*)o);
}


template <typename T>
struct Atom : public Object {
    T v;

    Atom(const T& i = 0) : v(i) {}

    size_t hash() const { return std::hash<T>()(v); }
    bool eq(Object* a) const { return v == get< Atom<T> >(a).v; }
    void print() const { std::cout << v; }
    void set(const std::vector<Object*>& s) { v = get< Atom<T> >(s[0]).v; }
    Object* clone() const { return new Atom<T>(v); }
};

typedef Atom<::Int> Int;
typedef Atom<::UInt> UInt;
typedef Atom<::Real> Real;
typedef Atom<std::string> String;


template <typename A>
size_t __array_index_do(const A& v, const Type& keytype, Object* key) {

    size_t i = v.size();

    switch (keytype.atom) {
    case Type::UINT:
    {
        i = get<UInt>(key).v;
        break;
    }
    case Type::INT:
    {
        ::Int z = get<Int>(key).v;
        if (z < 0)
            i = v.size() - z;
        else
            i = z;
        break;
    }
    case Type::REAL:
    {
        ::Real z = get<Real>(key).v;
        if (z >= 0.0 && z <= 1.0)
            i = v.size() * z;
    }
    default:
        break;
    }

    if (i >= v.size())
        throw std::runtime_error("Array index out of bounds");

    return i;
}


template <typename T>
struct ArrayAtom : public Object {
    std::vector<T> v;

    size_t hash() const {
        size_t ret = 0;
        for (const T& t : v) {
            ret += std::hash<T>()(t);
        }
        return ret;
    }

    bool eq(Object* a) const {
        return v == get< ArrayAtom<T> >(a).v;
    }

    void print() const {
        bool first = true;
        for (const T& x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << " ";
            }

            std::cout << x;
        }
    }

    void set(const std::vector<Object*>& s) {
        v = get< ArrayAtom<T> >(s[0]).v;
    }

    Object* clone() const {
        ArrayAtom<T>* ret = new ArrayAtom<T>;
        ret->v.assign(v.begin(), v.end());
        return ret;
    }

    void index(const Type& keytype, Object* key, Object*& out) const {

        size_t i = __array_index_do(v, keytype, key);

        Atom<T>& o = get< Atom<T> >(out);
        o->v = v[i];
    }
};

struct ArrayObject : public Object {
    std::vector<Object*> v;

    ~ArrayObject() {
        for (Object* x : v) {
            delete x;
        }
    }
    
    size_t hash() const {
        size_t ret = 0;
        for (Object* t : v) {
            ret += t->hash();
        }
        return ret;
    }

    bool eq(Object* a) const {

        const std::vector<Object*>& b = get<ArrayObject>(a).v;

        if (v.size() != b.size())
            return false;
        
        for (size_t i = 0; i < v.size(); ++i) {
            if (!(v[i]->eq(b[i])))
                return false;
        }

        return true;
    }

    void print() const {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << std::endl;
            }

            x->print();
        }
    }

    void set(const std::vector<Object*>& s) {
        v = get<ArrayObject>(s[0]).v;
    }

    Object* clone() const {

        ArrayObject* ret = new ArrayObject;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void index(const Type& keytype, Object* key, Object*& out) const {

        size_t i = __array_index_do(v, keytype, key);

        out = v[i];
    }
};

struct Tuple : public ArrayObject {

    void print() const {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << " ";
            }

            x->print();
        }
    }

    void set(const std::vector<Object*>& s) {

        for (size_t i = 0; i < v.size(); ++i) {
            v[i] = s[i];
        }
    }

    Object* clone() const {

        Tuple* ret = new Tuple;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void index(const Type& keytype, Object* key, Object*& out) const {
        out = v[get<UInt>(key).v];
    }
};

struct ObjectHash {
    size_t operator()(Object* o) const {
        return o->hash();
    }
};

struct ObjectEq {
    bool operator()(Object* a, Object* b) const {
        return a->eq(b);
    }
};

struct MapObject : public Object {

    typedef std::unordered_map<Object*, Object*, ObjectHash, ObjectEq> map_t;
    map_t v;

    ~MapObject() {
        for (const auto& x : v) {
            delete x.first;
            delete x.second;
        }
    }

    size_t hash() const {
        size_t ret = 0;
        for (const auto& t : v) {
            ret += t.first->hash();
            ret += t.second->hash();
        }
        return ret;
    }

    bool eq(Object* a) const {

        const map_t& b = get<MapObject>(a).v;

        if (v.size() != b.size())
            return false;

        auto i = v.begin();
        auto ie = v.end();
        auto j = b.begin();

        while (i != ie) {

            if (!(i->first->eq(j->first)) ||
                !(i->second->eq(j->second))) {

                return false;
            }
            
            ++i;
            ++j;
        }

        return true;
    }

    void print() const {
        bool first = true;
        for (const auto& x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << std::endl;;
            }

            x.first->print();
            std::cout << " ";
            x.second->print();
        }
    }

    void set(const std::vector<Object*>& s) {
        v = get<MapObject>(s[0]).v;
    }

    Object* clone() const {

        MapObject* ret = new MapObject;

        for (const auto& x : v) {
            Object* k = x.first->clone();
            Object* v = x.second->clone();
            ret->v[k] = v;
        }

        return ret;
    }
    
    void index(const Type& keytype, Object* key, Object*& out) const {

        auto i = v.find(key);

        if (i == v.end())
            throw std::runtime_error("Key is not in map");
        
        out = i->second;
    }

    void map(Object* key, Object* val) {

        auto i = v.find(key);

        if (i != v.end()) {
            delete i->second;
            i->second = val;

        } else {
            v[key] = val;
        }
    }
};


template <typename... U>
Object* make(const Type& t, U&&... u) {

    if (t.type == Type::ATOM) {
        switch (t.atom) {
        case Type::INT:
            return new Int(std::forward<U>(u)...);
        case Type::UINT:
            return new UInt(std::forward<U>(u)...);
        case Type::REAL:
            return new Real(std::forward<U>(u)...);
        case Type::STRING:
            return new String(std::forward<U>(u)...);
        }
    }

    if (!t.tuple || t.tuple->empty())
        return new Object();

    if (t.type == Type::TUP) {

        Tuple* ret = new Tuple(std::forward<U>(u)...);

        for (const Type& st : (*t.tuple)) {
            ret->v.push_back(make(st, std::forward<U>(u)...));
        }

        return ret;
    }

    if (t.type == Type::ARR) {

        const Type& s = (*t.tuple)[0];

        if (s.type == Type::ATOM) {

            switch (s.atom) {
            case Type::INT:
                return new ArrayAtom<::Int>(std::forward<U>(u)...);
            case Type::UINT:
                return new ArrayAtom<::UInt>(std::forward<U>(u)...);
            case Type::REAL:
                return new ArrayAtom<::Real>(std::forward<U>(u)...);
            case Type::STRING:
                return new ArrayAtom<std::string>(std::forward<U>(u)...);
            }
        }

        return new ArrayObject(std::forward<U>(u)...);
    }

    if (t.type == Type::MAP) {

        return new MapObject(std::forward<U>(u)...);
    }    

    throw std::runtime_error("Sanity error: cannot create object");
}

}


namespace funcs {


void cut(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& del = obj::get<obj::String>(args.v[1]).v;

    std::cout << "cut '" << str << "' '" << del << "'" << std::endl;
    
    size_t N = str.size();
    size_t M = del.size();

    size_t prev = 0;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;
    
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

void register_functions() {

    Functions& funcs = functions_init();
    
    funcs.add("cut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::cut);
}


struct Runtime {
    std::unordered_map<String,obj::Object*> vars;
    std::vector<obj::Object*> stack;

    void set_toplevel(obj::Object* o) {
        vars[strings().add("$")] = o;
    }
};


void execute_init(std::vector<Command>& commands) {

    for (auto& c : commands) {

        for (auto& cloptr : c.closure) {
            auto& clo = *cloptr;
            clo.object = obj::make(clo.type);
            execute_init(clo.code);
        }
            
        switch (c.cmd) {

        case Command::FUN:
        {
            c.object = obj::make(c.type);
            break;
        }
        case Command::VAL:
        {
            switch (c.arg.which) {
            case Atom::STRING:
                c.object = new obj::String(strings().get(c.arg.str));
                break;
            case Atom::INT:
                c.object = new obj::Int(c.arg.inte);
                break;
            case Atom::UINT:
                c.object = new obj::UInt(c.arg.uint);
                break;
            case Atom::REAL:
                c.object = new obj::Real(c.arg.real);
                break;
            }
            break;
        }
        case Command::IDX:
        {
            c.object = obj::make(c.type);
            break;
        }
        case Command::MAP:
        {
            c.object = obj::make(c.type);
            break;
        }
        default:
            break;
        }
    }
}

void execute(std::vector<Command>& commands, Runtime& r);

obj::Object* _exec_closure(Runtime& rsub, Command& c, size_t n) {

    Command::Closure& closure = *(c.closure[n]);
    execute(closure.code, rsub);
            
    obj::Object* o = closure.object;
    o->set(rsub.stack);

    rsub.stack.clear();

    return o;
}


void execute(std::vector<Command>& commands, Runtime& r) {
    
    for (Command& c : commands) {
        switch (c.cmd) {

        case Command::FUN:
        {
            Runtime rsub;
            rsub.vars = r.vars;
            obj::Object* arg = _exec_closure(rsub, c, 0);

            ((Functions::func_t)c.function)(arg, c.object);

            r.stack.push_back(c.object);
            break;
        }
        case Command::VAR:
        {
            r.stack.push_back(r.vars[c.arg.str]);
            break;
        }
        case Command::VAW:
        {
            r.vars[c.arg.str] = r.stack.back();
            r.stack.pop_back();
            break;
        }
        case Command::VAL:
        {
            r.stack.push_back(c.object);
            break;
        }
        case Command::IDX:
        {
            Runtime rsub;
            rsub.vars = r.vars;

            obj::Object* key = _exec_closure(rsub, c, 0);

            obj::Object* cont = r.stack.back();
            obj::Object* val = c.object;

            cont->index(c.closure[0]->type, key, val);
            
            r.stack.pop_back();
            r.stack.push_back(val);
            break;
        }
        case Command::MAP:
        {
            Runtime rsub;
            rsub.vars = r.vars;

            obj::Object* src = _exec_closure(rsub, c, 2);
            obj::Object* dst = c.object;
            
            while (1) {
                bool ok;
                Object* next;
                
                src->next(next, ok);

                rsub.set_toplevel(next);

                obj::Object* key = _exec_closure(rsub, c, 0);
                obj::Object* val = _exec_closure(rsub, c, 1);

                dst->map(key->clone(), val->clone());
                
                if (!ok) break;
            }

            break;
        }
        default:
            break;
        }
    }        
}


void print(const Runtime& r) {

    for (obj::Object* o : r.stack) {
        o->print();
        std::cout << std::endl;
    }    
}

        
int main(int argc, char** argv) {

    try {

        if (argc != 2 && argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <expression> [arg]" << std::endl;
            return 1;
        }

        std::string program(argv[1]);
        std::string inputs;

        if (argc == 3) {
            inputs.assign(argv[2]);
        }

        register_functions();

        TypeResult typer;
        std::vector<Command> commands;

        parse(program.begin(), program.end(), typer, commands);

        Runtime res;

        obj::String toplevel;
        toplevel.v.swap(inputs);
        res.set_toplevel(&toplevel);

        execute(commands, res);

        print(res);
        
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
