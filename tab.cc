
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

#include <regex>

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
        SEQ,
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
    
    Type(types_t t = NONE) : type(t), atom(INT) {}

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
        case SEQ: ret += "SEQ"; break;
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

        ARR,
        MAP,
        FUN,
        SEQ,
        TUP
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
        case ARR: return "ARR";
        case MAP: return "MAP";
        case FUN: return "FUN";
        case SEQ: return "SEQ";
        case TUP: return "TUP";
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

Type wrap_seq(const Type& t) {

    Type ret(Type::SEQ);
    
    if (t.type == Type::ARR) {

        ret.push(t.tuple->at(0));

    } else if (t.type == Type::MAP) {

        Type tmp(Type::TUP);
        tmp.push(t.tuple->at(0));
        tmp.push(t.tuple->at(1));

        ret.push(tmp);

    } else {
        ret.push(t);
    }

    return ret;
}

Type unwrap_seq(const Type& t) {

    if (t.type != Type::SEQ || !t.tuple || t.tuple->size() != 1)
        throw std::runtime_error("Sanity error: expected a sequence generator");

    return t.tuple->at(0);
}


struct TypeRuntime {

    std::vector<Type> stack;
    std::unordered_map<String, Type> vars;
};


Type infer_types(std::vector<Command>& commands, const Type& toplevel, TypeRuntime& typer);

const Type& infer_closure(Command& c, size_t n, const Type& toplevel, TypeRuntime& typer,
                          const std::string& name, bool do_unwrap_seq = false) {

    if (n >= c.closure.size())
        throw std::runtime_error("Sanity error, asked to infer non-existing closure.");
    
    auto& cc = *(c.closure[n]);
    cc.type = infer_types(cc.code, toplevel, typer);

    if (do_unwrap_seq) {
        cc.type = unwrap_seq(cc.type);
    }
    
    return cc.type;
}

Type infer_tup_generator(Command& c, Type toplevel, const TypeRuntime& _tr, const std::string& name) {

    if (c.closure.size() != 1)
        throw std::runtime_error("Sanity error, " + name + " is not a closure.");

    TypeRuntime typer;
    typer.vars = _tr.vars;

    return infer_closure(c, 0, toplevel, typer, name);
}

Type infer_arr_generator(Command& c, Type toplevel, const TypeRuntime& _tr, const std::string& name) {

    if (c.closure.size() != 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    TypeRuntime typer;
    typer.vars = _tr.vars;

    toplevel = infer_closure(c, 1, toplevel, typer, name, true);
        
    const Type& t = infer_closure(c, 0, toplevel, typer, name);
    
    Type ret(Type::ARR);
    ret.push(t);

    return ret;
}

Type infer_map_generator(Command& c, Type toplevel, const TypeRuntime& _tr, const std::string& name) {

    if (c.closure.size() != 3)
        throw std::runtime_error("Sanity error, generator is not a map closure.");

    TypeRuntime typer;
    typer.vars = _tr.vars;

    toplevel = infer_closure(c, 2, toplevel, typer, name, true);
    
    const Type& tk = infer_closure(c, 0, toplevel, typer, name);
    const Type& tv = infer_closure(c, 1, toplevel, typer, name);

    Type ret(Type::MAP);
    ret.push(tk);
    ret.push(tv);

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

Type infer_idx_generator(const Type& tv, Command& c, const Type& toplevel, const TypeRuntime& typer,
                         const std::string& name) {
            
    Type ti = infer_tup_generator(c, toplevel, typer, "structure index");

    if (tv.type == Type::TUP) {

        auto& cl = *(c.closure.at(0));
                
        if (cl.code.size() == 1) {

            const auto& v = cl.code[0];

            if (v.cmd == Command::VAL && (v.arg.which == Atom::INT || v.arg.which == Atom::UINT)) {

                UInt i = v.arg.uint;

                if (tv.tuple && i < tv.tuple->size()) {

                    return tv.tuple->at(i);
                }
            }
        }

        throw std::runtime_error("Indexing tuples is only possible with integer literals.");

    } else if (tv.type == Type::ARR) {
                
        if (!check_numeric(ti))
            throw std::runtime_error("Arrays must be accessed with numeric index.");

        return value_type(tv);

    } else if (tv.type == Type::MAP) {
                
        if (ti != mapped_type(tv))
            throw std::runtime_error("Invalid key type when accessing map: key is " +
                                     Type::print(mapped_type(tv)) + ", acessing with " +
                                     Type::print(ti));

        return value_type(tv);
    }

    throw std::runtime_error("Cannot index a scalar value.");
}


Type infer_types(std::vector<Command>& commands, const Type& toplevel, TypeRuntime& typer) {

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

        case Command::IDX:
        {
            Type tv = stack.back();
            stack.pop_back();

            Type t = infer_idx_generator(tv, c, toplevel, typer, "structure index");
            stack.emplace_back(t);
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
        case Command::SEQ:
        {
            Type ti = stack.back();
            Type to = wrap_seq(ti);
            stack.pop_back();
            stack.emplace_back(to);
            break;
        }
        case Command::TUP:
        {
            if (stack.size() == 0)
                throw std::runtime_error("Sanity error: empty tuple");

            if (stack.size() > 1) {
                
                Type t(Type::TUP);

                for (const Type& ti : stack) {
                    t.push(ti);
                }

                stack.clear();
                stack.emplace_back(t);
            }
            break;
        }
        }

        if (c.cmd != Command::VAW) {
            c.type = stack.back();
        }
    }


    if (stack.size() == 0)
        throw std::runtime_error("Empty sequences are not allowed.");

    if (stack.size() == 1) {

        return stack[0];
        
    } else {

        Type ret(Type::TUP);
        
        for (const auto& c : stack) {
            ret.push(c);
        }

        return ret;
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
Type parse(I beg, I end, TypeRuntime& typer, std::vector<Command>& commands) {

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
    auto y_close_seq = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); stack.push(Command::SEQ); });
    
    auto y_true = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, (Int)1); });

    auto y_default_from = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, strings().add("$")); });
    
    auto x_from =
        (((axe::r_lit(':') >> y_mark) & x_expr) |
         (axe::r_empty() >> y_mark >> y_default_from)) >> y_close_seq >> y_close_arg;

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

    x_expr_atom = x_expr_bit;

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

    Type ret = infer_types(stack.stack, Type(Type::STRING), typer);

    stack.print();
    std::cout << Type::print(ret) << std::endl;

    commands.swap(stack.stack);

    return ret;
}


/** ** **/


namespace obj {

struct Object {

    typedef std::function<Object*(Object*,bool&)> iterator_t;
    
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

    virtual iterator_t iter() const {
        throw std::runtime_error("Object iteration not implemented");
    }
};

template <typename T>
T& get(const Object* o) {
    return *((T*)o);
}


template <typename T>
struct Atom : public Object {
    T v;

    Atom(const T& i = T()) : v(i) {}

    size_t hash() const { return std::hash<T>()(v); }
    bool eq(Object* a) const { return v == get< Atom<T> >(a).v; }
    void print() const { std::cout << v; }
    void set(const std::vector<Object*>& s) { v = get< Atom<T> >(s[0]).v; }
    Object* clone() const { return new Atom<T>(v); }

    iterator_t iter() const { return [this](Object* i, bool& ok) { ok = false; return (Object*)this; }; }
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
                std::cout << std::endl;
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
        o.v = v[i];
    }

    void map(Object* val, Object*) {

        v.push_back(get< Atom<T> >(val).v);
    }

    iterator_t iter() const {

        typename std::vector<T>::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty array");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Atom<T>& x = get< Atom<T> >(i);
            x.v = *ite;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return i;
        };
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

    void map(Object* val, Object*) {

        v.push_back(val);
    }
        
    iterator_t iter() const {

        typename std::vector<Object*>::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty array");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Object* ret = *ite;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return ret;
        };
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

    iterator_t iter() const { return [this](Object* i, bool& ok) { ok = false; return (Object*)this; }; }
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

    iterator_t iter() const {

        typename map_t::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty map");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Tuple& x = get<Tuple>(i);
            x.v[0] = ite->first;
            x.v[1] = ite->second;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return i;
        };
    }
};

struct Sequencer : public Object {

    iterator_t v;
    
    void wrap(Object* i) {
        v = i->iter();
    }

    Object* next(Object* holder, bool& ok) {
        return v(holder, ok);
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

    } else if (t.type == Type::ARR) {

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

    } else if (t.type == Type::MAP) {

        return new MapObject(std::forward<U>(u)...);

    } else if (t.type == Type::SEQ) {

        return new Sequencer;
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
        }
    }

    v.emplace_back(str.begin() + prev, str.end());
}

void grep(const obj::Object* in, obj::Object*& out) {

    static std::unordered_map<std::string, std::regex> _cache;

    
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    auto i = _cache.find(regex);

    if (i == _cache.end()) {
        i = _cache.insert(i, std::make_pair(regex, std::regex(regex, std::regex_constants::optimize)));
    }

    std::regex& r = i->second;

    std::sregex_iterator iter(str.begin(), str.end(), r);
    std::sregex_iterator end;

    while (iter != end) {

        if (iter->size() == 1) {

            v.emplace_back(iter->str());

        } else if (iter->size() > 1) {
            auto subi = iter->begin();
            auto sube = iter->end();
            ++subi;
            
            while (subi != sube) {
                v.emplace_back(subi->str());
                ++subi;
            }
        }

        ++iter;
    }
}

}

void register_functions() {

    Functions& funcs = functions_init();
    
    funcs.add("cut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::cut);

    funcs.add("grep",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::grep);
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

        case Command::VAL:
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

        case Command::VAW:
            break;
            
        default:
            c.object = obj::make(c.type);
            break;
        }
    }
}

void execute_run(std::vector<Command>& commands, Runtime& r);

obj::Object* _exec_closure(Runtime& rsub, Command& c, size_t n) {

    Command::Closure& closure = *(c.closure[n]);
    execute_run(closure.code, rsub);
            
    obj::Object* o = closure.object;
    o->set(rsub.stack);

    rsub.stack.clear();

    return o;
}

obj::Sequencer& _exec_seq_closure(Runtime& rsub, Command& c, size_t n, obj::Object*& ite) {

    Command::Closure& closure = *(c.closure[n]);
    execute_run(closure.code, rsub);

    ite = closure.object;
    obj::Object* ret = rsub.stack.back();
    rsub.stack.clear();
    return obj::get<obj::Sequencer>(ret);
}


void execute_run(std::vector<Command>& commands, Runtime& r) {
    
    for (Command& c : commands) {
        std::cout << " ~ " << Command::print(c.cmd) << std::endl;
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
        case Command::TUP:
        {
            obj::Object* o = c.object;
            o->set(r.stack);
            r.stack.clear();
            r.stack.push_back(o);
            break;
        }
        case Command::SEQ:
        {
            obj::Object* src = r.stack.back();
            r.stack.pop_back();

            obj::Sequencer& seq = obj::get<obj::Sequencer>(c.object);
            seq.wrap(src);
            r.stack.push_back(c.object);
            break;
        }
        case Command::ARR:
        {
            Runtime rsub;
            rsub.vars = r.vars;

            obj::Object* ite;
            obj::Sequencer& seq = _exec_seq_closure(rsub, c, 1, ite);
            obj::Object* dst = c.object;
            
            while (1) {
                bool ok;
                
                obj::Object* next = seq.next(ite, ok);

                rsub.set_toplevel(next);

                obj::Object* val = _exec_closure(rsub, c, 0);

                dst->map(val->clone(), nullptr);
                
                if (!ok) break;
            }

            r.stack.push_back(dst);

            break;
        }
        case Command::MAP:
        {
            Runtime rsub;
            rsub.vars = r.vars;

            obj::Object* ite;
            obj::Sequencer& seq = _exec_seq_closure(rsub, c, 2, ite);
            obj::Object* dst = c.object;
            
            while (1) {
                bool ok;
                
                obj::Object* next = seq.next(ite, ok);

                rsub.set_toplevel(next);

                obj::Object* key = _exec_closure(rsub, c, 0);
                obj::Object* val = _exec_closure(rsub, c, 1);

                dst->map(key->clone(), val->clone());
                
                if (!ok) break;
            }

            r.stack.push_back(dst);

            break;
        }

        case Command::NOT:
        case Command::NEG:
        case Command::EXP:
        case Command::MUL:
        case Command::DIV:
        case Command::MOD:
        case Command::ADD:
        case Command::SUB:
        case Command::AND:
        case Command::OR:
        case Command::XOR:
        
        default:
            break;
        }
    }        
}


void execute(std::vector<Command>& commands, const Type& type, const std::string& inputs) {

    Runtime rt;

    obj::String* toplevel = new obj::String(inputs);
    rt.set_toplevel(toplevel);

    execute_init(commands);
    execute_run(commands, rt);

    obj::Object* res;
    
    if (rt.stack.size() == 1) {
        res = rt.stack[0];

    } else {
        res = obj::make(type);
        res->set(rt.stack);
    }

    res->print();
    std::cout << std::endl;
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

        std::vector<Command> commands;
        TypeRuntime typer;

        Type finaltype = parse(program.begin(), program.end(), typer, commands);

        execute(commands, finaltype, inputs);
        
    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
