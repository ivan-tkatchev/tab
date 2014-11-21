
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <map>

#include <iostream>

#include "axe.h"


typedef long Int;
typedef unsigned long UInt;
typedef double Real;
typedef std::string String;

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

    Atom(Int i = 0) : which(INT), inte(i) { }
    Atom(UInt i) : which(UINT), uint(i)  { }
    Atom(Real i) : which(REAL), real(i)  { }

    Atom(const String& i) : which(STRING) {
        new (&str) String();
        str.assign(i.begin(), i.end());
        std::cout << "  ..ctr.. " << str << std::endl;
    }

    void copy(const Atom& a) {

        if (this == &a)
            return;

        if (a.which == 3)
            std::cout << " ~~ " << a.str << std::endl;
        
        std::cout << "COPY " << which << ":" << print() << " <- " << a.which << ":" << a.print() << std::endl;
        
        if (which == STRING)
            str.~String();

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
            new (&str) String();
            str.assign(a.str.begin(), a.str.end());
            break;
        };

        which = a.which;
    }
    
    Atom(const Atom& a) {

        std::cout << " ..copy ctr.. " << std::endl;
        
        copy(a);
    }

    ~Atom() {
        if (which == STRING)
            str.~String();
    }

    Atom& operator=(const Atom& a) {
        std::cout << " ..assign op.. " << std::endl;
        copy(a);
        return *this;
    }
    
    std::string print() const {
        switch (which) {
        case INT:
            return std::to_string(inte);
        case UINT:
            return std::to_string(uint);
        case REAL:
            return std::to_string(real);
        case STRING:
            return str;
        }
        return ":~(";
    }
};
   
struct Type {

    enum types_t {
        ATOM,
        ARR,
        SET,
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
    std::vector<atom_types_t> arg1;
    std::vector<atom_types_t> arg2;

    Type(types_t t = NONE) : type(t) {}

    Type(types_t t, atom_types_t a) : type(t) {
        arg1.push_back(a);
    }
        
    Type(const Atom& a) : type(ATOM) {
        switch (a.which) {
        case INT: arg1.push_back(INT); break;
        case UINT: arg1.push_back(UINT); break;
        case REAL: arg1.push_back(REAL); break;
        case STRING: arg1.push_back(STRING); break;
        }
    }

    Type(types_t t, const std::vector<atom_types_t>& a1) : type(t), arg1(a1) {}

    Type(types_t t, const std::vector<atom_types_t>& a1, const std::vector<atom_types_t>& a2) : type(t), arg1(a1), arg2(a2) {}

    bool operator!=(const Type& t) {
        return !(t.type == type && t.arg1 == arg1 && t.arg2 == arg2);
    }
    
    static std::string print(const Type& t) {
        std::string ret;

        switch (t.type) {
        case NONE: ret += "NONE("; break;
        case ATOM: ret += "ATOM("; break;
        case ARR:  ret += "ARRAY("; break;
        case SET:  ret += "SET("; break;
        case MAP:  ret += "MAP("; break;
        }

        for (auto z : t.arg1) {
            switch (z) {
            case INT: ret += " INT"; break;
            case UINT: ret += " UINT"; break;
            case REAL: ret += " REAL"; break;
            case STRING: ret += " STRING"; break;
            }
        }

        if (t.arg2.size() > 0) {
            ret += " ;";

            for (auto z : t.arg2) {
                switch (z) {
                case INT: ret += " INT"; break;
                case UINT: ret += " UINT"; break;
                case REAL: ret += " REAL"; break;
                case STRING: ret += " STRING"; break;
                }
            }
        }

        ret += " )";
        return ret;
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
        SET,
        MAP,
        FUN
    };

    cmd_t cmd;

    Atom arg;

    typedef std::shared_ptr< std::vector<Command> > closure_t;

    std::vector<closure_t> closure;

    Type type;
    
    Command(cmd_t c = VAL) : cmd(c) {}

    template <typename T>
    Command(cmd_t c, const T& t) : cmd(c), arg(t) {}

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
        case SET: return "SET";
        case MAP: return "MAP";
        case FUN: return "FUN";
        }
        return ":~(";
    }
};

struct Functions {

    std::map<String, Type> types;

    Functions() {

        add_type("sin",  Type(Type::ATOM, Type::REAL));
        add_type("cos",  Type(Type::ATOM, Type::REAL));
        //add_type(std::string("tan"),  Type(Type::ATOM, Type::REAL));
        add_type("sqrt", Type(Type::ATOM, Type::REAL));
        add_type("exp",  Type(Type::ATOM, Type::REAL));
        add_type("log",  Type(Type::ATOM, Type::REAL));

        add_type("cut",  Type(Type::ARR, Type::STRING));
    }

    void add_type(const String& s, const Type& t) {
        types.insert(types.end(), std::make_pair(s, t));
    }
};

const Functions& functions() {
    static Functions ret;
    return ret;
}

Type function_type(const std::string& name) {

    const Functions& f = functions();

    auto i = f.types.find(name);
    
    if (i == f.types.end())
        throw std::runtime_error("Unknown function: '" + name + "'");

    return i->second;
}
    
bool check_integer(const Type& t) {
    return (t.type == Type::ATOM && t.arg1.size() == 1 && (t.arg1[0] == Type::INT || t.arg1[0] == Type::UINT));
}

bool check_real(const Type& t) {
    return (t.type == Type::ATOM && t.arg1.size() == 1 && t.arg1[0] == Type::REAL);
}

bool check_numeric(const Type& t) {
    return (t.type == Type::ATOM && t.arg1.size() == 1 && (t.arg1[0] == Type::INT || t.arg1[0] == Type::UINT || t.arg1[0] == Type::REAL));
}

bool check_string(const Type& t) {
    return (t.type == Type::ATOM && t.arg1.size() == 1 && t.arg1[0] == Type::STRING);
}

void handle_real_operator(std::vector<Type>& stack, const std::string& name) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    stack.emplace_back(Type::ATOM, Type::REAL);
}

void handle_int_operator(std::vector<Type>& stack, const std::string& name) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_integer(t1) || !check_integer(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-integer value.");

    auto a1 = t1.arg1[0];
    auto a2 = t2.arg1[0];

    if (a1 == Type::UINT && a2 == Type::UINT) {
        stack.emplace_back(Type::ATOM, Type::UINT);

    } else {
        stack.emplace_back(Type::ATOM, Type::INT);
    }
}

void handle_poly_operator(std::vector<Type>& stack, const std::string& name, bool always_int = false) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    auto a1 = t1.arg1[0];
    auto a2 = t2.arg1[0];

    if (a1 == Type::REAL || a2 == Type::REAL) {
        stack.emplace_back(Type::ATOM, Type::REAL);

    } else if (!always_int && a1 == Type::UINT && a2 == Type::UINT) {
        stack.emplace_back(Type::ATOM, Type::UINT);

    } else {
        stack.emplace_back(Type::ATOM, Type::INT);
    }
}

Type homo_type(const std::vector<Command>& commands, const std::string& name) {

    if (commands.empty())
        throw std::runtime_error("Empty sequences are not allowed.");

    Type ret;

    if (commands.size() == 1) {
        ret.type = Type::ATOM;

    } else {
        ret.type = Type::ARR;
    }
        
    for (const auto& c : commands) {

        if (c.type.type != Type::ATOM) {
            throw std::runtime_error("In " + name + ": nested sequences are not allowed.");
        }

        if (c.type.arg1.size() != 1)
            throw std::runtime_error("Sanity error.");
        
        ret.arg1.emplace_back(c.type.arg1[0]);
    }

    return ret;
}

void infer_types(std::vector<Command>& commands, const Type& toplevel);

Type infer_generator(Command& c, Type toplevel, const std::string& name) {

    if (c.closure.size() < 1 || c.closure.size() > 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    if (c.closure.size() == 2) {

        auto& cfrom = *(c.closure.at(1));
        infer_types(cfrom, toplevel);

        toplevel = homo_type(cfrom, name);
    }
    
    auto& cto = *(c.closure.at(0));
    infer_types(cto, toplevel);
    
    return homo_type(cto, name);
}

Type infer_map_generator(Command& c, Type toplevel, const std::string& name) {

    if (c.closure.size() < 2 || c.closure.size() > 3)
        throw std::runtime_error("Sanity error, generator is not a map closure.");

    if (c.closure.size() == 3) {

        auto& cfrom = *(c.closure.at(2));
        infer_types(cfrom, toplevel);

        toplevel = homo_type(cfrom, name);
    }
    
    auto& cto_k = *(c.closure.at(0));
    infer_types(cto_k, toplevel);

    auto& cto_v = *(c.closure.at(1));
    infer_types(cto_v, toplevel);
    
    Type out1 = homo_type(cto_k, name);
    Type out2 = homo_type(cto_v, name);

    out1.arg2.swap(out2.arg2);
    return out1;
}

Type value_type(const Type& t) {

    if (t.arg1.size() == 0)
        throw std::runtime_error("Sanity error.");

    Type ret = t;

    if (ret.arg1.size() == 1) {
        ret.type = Type::ATOM;
    } else {
        ret.type = Type::ARR;
    }

    return ret;
}

Type mapped_type(const Type& t) {

    if (t.arg2.size() == 0)
        throw std::runtime_error("Sanity error.");

    Type ret = t;

    ret.arg1.swap(ret.arg2);
    ret.arg2.clear();

    if (t.arg1.size() == 1) {
        ret.type = Type::ATOM;
    } else {
        ret.type = Type::ARR;
    }

    return ret;
}

void infer_types(std::vector<Command>& commands, const Type& toplevel) {

    std::vector<Type> stack;
    std::map<String, Type> vars;

    vars["$"] = toplevel;
    
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
                throw std::runtime_error("Use of undefined variable: " + c.arg.str);

            stack.emplace_back(i->second);
            break;
        }
        
        case Command::NOT:
            stack.pop_back();
            stack.emplace_back(Type::ATOM, Type::INT);
            break;

        case Command::NEG:
        {
            Type t = stack.back();

            if (!check_integer(t)) 
                throw std::runtime_error("Use of '~' numeric operator on something other than integer or unsigned integer.");

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

            stack.emplace_back(Type::ARR, Type::STRING);
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
                
                if (!(ti.type == Type::ARR && ti.arg1.size() == 1 && (ti.arg1[0] == Type::INT || ti.arg1[0] == Type::UINT)))
                    throw std::runtime_error("Arrays must be accessed with numeric index.");

                
                stack.emplace_back(value_type(tv));
                break;

            case Type::SET:

                stack.emplace_back(Type::ATOM, Type::INT);

                if (ti != value_type(tv))
                    throw std::runtime_error("Invalid key type when accessing set.");
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
            Type t = infer_generator(c, toplevel, "array");
            stack.emplace_back(Type::ARR, t.arg1);
            break;
        }

        case Command::SET:
        {
            Type t = infer_generator(c, toplevel, "set");
            stack.emplace_back(Type::SET, t.arg1);
            break;
        }

        case Command::MAP:
        {
            Type t = infer_map_generator(c, toplevel, "map");
            stack.emplace_back(Type::MAP, t.arg1, t.arg2);
            break;
        }

        case Command::FUN:
        {
            Type t = function_type(c.arg.str);
            stack.emplace_back(t);
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

    void push(Command::cmd_t c) { std::cout << "  push0" << std::endl; std::cout << Command::print(c) << std::endl; std::cout << stack.size() << std::endl;
        //stack.emplace_back(c);
        for (const auto& i : stack) {
            std::cout << "!!! " << Command::print(i.cmd) << " " << i.arg.which << " " << i.closure.size() << std::endl;
            for (const auto& ii : i.closure) {
                for (const auto& iii : *ii) {
                    std::cout << "   . " << Command::print(iii.cmd) << " " << iii.arg.which;
                    if (iii.arg.which == 3) std::cout << iii.arg.str;
                    std::cout << std::endl;
                }
            }
        }
        stack.emplace_back(c);
        std::cout << "PUSH0" << std::endl; }

    template <typename T>
    void push(Command::cmd_t c, const T& t) { std::cout << "  push1" << std::endl; stack.emplace_back(c, t); std::cout << "PUSH1" << std::endl; }

    Atom& back() {

        if (stack.empty())
            throw std::runtime_error("Sanity check.");

        return stack.back().arg;
    }

    std::vector< std::pair<size_t,String> > _mark;
    std::vector< String> names;
    
    void mark() {
        _mark.emplace_back(stack.size(), String());
    }

    void mark(const String& n) {
        _mark.emplace_back(stack.size(), n);
    }
    
    void close(Command::cmd_t cmd) {

        auto m = _mark.back();
        _mark.pop_back();

        std::cout << "CLOSE " << m.first << " " << stack.size() << std::endl;
        
        auto c = std::make_shared< std::vector<Command> >(stack.begin() + m.first, stack.end());
        stack.erase(stack.begin() + m.first, stack.end());

        if (m.second.empty()) {
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

        std::cout << "CLOSE_ " << m.first << " " << stack.size() << std::endl;
                
        auto c = std::make_shared< std::vector<Command> >(stack.begin() + m.first, stack.end());
        stack.erase(stack.begin() + m.first, stack.end());

        stack.back().closure.resize(stack.back().closure.size() + 1);
        stack.back().closure.back().swap(c);
    }

    static void print(const std::vector<Command>& c, size_t level) {

        for (const auto& i : c) {
            std::cout << " " << std::string(level*2, ' ')
                      << Command::print(i.cmd) << " " << i.arg.which << ": " << i.arg.print() << std::endl;

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
void parse(I beg, I end) {

    Stack stack;
    
    
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

    auto y_string = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, std::string()); });
    auto y_quotedchar = axe::e_ref([&](I b, I e) {

            std::string z(b, e);

            if (z == "t") {
                stack.back().str += '\t';
            } else if (z == "n") {
                stack.back().str += '\n';
            } else if (z == "e") {
                stack.back().str += '\e';
            } else {
                stack.back().str += z;
            }
        });
    auto y_char = axe::e_ref([&](I b, I e) { stack.back().str += *b; });
    
    auto x_quotedchar = axe::r_lit("\\") > (axe::r_any() >> y_quotedchar);
    auto x_char1 = x_quotedchar | (axe::r_any() - axe::r_lit('\\') - axe::r_lit('"')) >> y_char;
    auto x_char2 = x_quotedchar | (axe::r_any() - axe::r_lit('\\') - axe::r_lit('\'')) >> y_char;
    auto x_string =
        (axe::r_lit('"') >> y_string & axe::r_many(x_char1,0) & axe::r_lit('"')) |
        (axe::r_lit('\'') >> y_string & axe::r_many(x_char2,0) & axe::r_lit('\''));

    auto x_literal = x_float | x_uint | x_int | x_string;

    auto x_var = axe::r_lit('$') | (axe::r_alpha() & axe::r_many(axe::r_alnum() | axe::r_lit('_'),0));

    auto y_mark = axe::e_ref([&](I b, I e) { stack.mark(); std::cout << "Mark" << std::endl; });
    auto y_mark_name = axe::e_ref([&](I b, I e) { stack.mark(std::string(b, e)); std::cout << "marked funname " << std::string(b, e) << std::endl; });
    auto y_close_arg = axe::e_ref([&](I b, I e) { stack.close(); std::cout << "Close arg" << std::endl; });
    auto y_close_set = axe::e_ref([&](I b, I e) { stack.close(Command::SET); std::cout << "Close set" << std::endl; });
    auto y_close_arr = axe::e_ref([&](I b, I e) { stack.close(Command::ARR); std::cout << "Close arr" << std::endl; });
    auto y_close_map = axe::e_ref([&](I b, I e) { stack.close(Command::MAP); std::cout << "Close map" << std::endl; });
    auto y_close_fun = axe::e_ref([&](I b, I e) { stack.close(Command::FUN); std::cout << "closed fun" << std::endl; });
    
    auto x_from = ~((axe::r_lit(':') >> y_mark) & (x_expr >> y_close_arg));
    
    auto x_set =
        (axe::r_lit('{') >> y_mark) & (x_expr >> y_close_set) & x_from & axe::r_lit('}');
    
    auto x_array =
        (axe::r_lit('[') >> y_mark) & (x_expr >> y_close_arr) & x_from & axe::r_lit(']');
    
    auto x_map =
        (axe::r_lit('{')  >> y_mark) & (x_expr >> y_close_map) &
        (axe::r_lit("->") >> y_mark) & (x_expr >> y_close_arg) & x_from & axe::r_lit('}');
    
    auto x_funcall =
        (x_var >> y_mark_name) & x_ws & axe::r_lit('(') & (x_expr >> y_close_fun) & axe::r_lit(')') >> axe::e_ref([](I b, I e) { std::cout << "Funcall done" << std::endl; });

    auto y_var_read = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, std::string(b, e)); std::cout << "read var " << std::string(b, e) << std::endl; });
    
    auto x_var_read = x_var >> y_var_read;

    auto x_expr_bottom =
        x_ws &
        (x_literal | x_funcall | x_var_read | x_set | x_array | x_map |
         (axe::r_lit('(') & x_expr_atom & axe::r_lit(')'))) &
        x_ws;

    auto y_expr_idx = axe::e_ref([&](I b, I e) { stack.push(Command::IDX); });
    
    auto x_expr_idx =
        x_expr_bottom & ~(x_array >> y_expr_idx) >> axe::e_ref([](I b, I e) { std::cout << "x_expr_idx" << std::endl; });
    
    auto y_expr_not = axe::e_ref([&](I b, I e) { stack.push(Command::NOT); });
    auto y_expr_neg = axe::e_ref([&](I b, I e) { stack.push(Command::NEG); });
    
    auto x_expr_neg =
        (axe::r_any("!") & x_expr_atom >> y_expr_not) |
        (axe::r_any("~") & x_expr_atom >> y_expr_neg) |
        x_expr_idx;

    auto y_expr_exp = axe::e_ref([&](I b, I e) { stack.push(Command::EXP); });
    
    auto x_expr_exp =
        x_expr_neg & ~(axe::r_lit("**") & x_expr_atom >> y_expr_exp) >> axe::e_ref([](I b, I e) { std::cout << "x_expr_exp" << std::endl; });

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
        x_expr_bit & ~(axe::r_lit("~") & x_ws & x_string >> y_expr_regex) >> axe::e_ref([](I b, I e) { std::cout << "x_expr_regex" << std::endl; });

    x_expr_atom = x_expr_regex >> axe::e_ref([](I b, I e) { std::cout << "x_expr_atom" << std::endl; });

    auto y_expr_assign_var = axe::e_ref([&](I b, I e) { stack.names.emplace_back(b, e); std::cout << "Assign " << std::string(b, e) << std::endl; });
    auto y_expr_assign = axe::e_ref([&](I b, I e) { stack.push(Command::VAW, stack.names.back()); stack.names.pop_back(); std::cout << "Assign OK" << std::endl; });

    auto y_no_assign = axe::e_ref([&](I b, I e) { stack.names.pop_back(); std::cout << "No assign" << std::endl; });
    
    auto x_expr_assign = 
        (x_ws &
         (x_var >> y_expr_assign_var) &
         x_ws &
         (axe::r_lit('=') | r_fail(y_no_assign)) &
         x_ws &
         (x_expr_atom >> y_expr_assign)) | 
        x_expr_atom >> axe::e_ref([](I b, I e) { std::cout << "x_expr_assign" << std::endl; });

    auto x_expr_seq = x_expr_assign & *(axe::r_lit(',') & x_expr_assign);

    x_expr = x_expr_seq >> axe::e_ref([](I b, I e) { std::cout << "EXPR: " << std::string(b, e) << std::endl; });

    auto x_main = x_expr & axe::r_end();

    auto x_go = x_main >> axe::e_ref([](I b, I e) { std::cout << "DONE" << std::endl; }) |
        axe::r_fail([](I b, I e) {
                throw std::runtime_error("Syntax error, unparsed input: \"" + std::string(b, e) + "\"");
            });
    
    x_go(beg, end);

    stack.print();

    infer_types(stack.stack, Type(Type::ATOM, Type::STRING));

    for (const auto& c : stack.stack) {
        std::cout << Type::print(c.type) << std::endl;
    }
}

int main(int argc, char** argv) {

    try {

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <expression>" << std::endl;
            return 1;
        }

        std::string inp(argv[1]);
        parse(inp.begin(), inp.end());

    } catch (std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "UNKNOWN ERROR." << std::endl;
        return 1;
    }

    return 0;
}
