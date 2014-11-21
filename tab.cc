
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

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
    Atom(UInt i) : which(UINT), uint(i)  {}
    Atom(Real i) : which(REAL), real(i)  {}

    Atom(const String& i) : which(STRING) {
        new (&str) String(i);
    }

    void copy(const Atom& a) {
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
            new (&str) String(a.str);
            break;
        };

        which = a.which;
    }
    
    Atom(const Atom& a) {
        copy(a);
    }

    ~Atom() {
        if (which == STRING)
            str.~String();
    }

    Atom& operator=(const Atom& a) {
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
   

struct Command {

    enum cmd_t {
        VAL,
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
    
    Command(cmd_t c) : cmd(c) {}

    template <typename T>
    Command(cmd_t c, const T& t) : cmd(c), arg(t) {}

    static std::string print(cmd_t c) {
        switch (c) {
        case VAL: return "VAL";
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
        case REGEX: return "REGEX";
        case ARR: return "ARR";
        case SET: return "SET";
        case MAP: return "MAP";
        case FUN: return "FUN";
        }
        return ":~(";
    }
};

struct Stack {

    std::vector<Command> stack;

    void push(Command::cmd_t c) { stack.emplace_back(c); }

    template <typename T>
    void push(Command::cmd_t c, const T& t) { stack.emplace_back(c, t); }

    Atom& back() {
        return stack.back().arg;
    }

    std::vector< std::pair<size_t,String> > _mark;
    
    void mark() {
        _mark.emplace_back(stack.size(), String());
    }

    void mark(const String& n) {
        _mark.emplace_back(stack.size(), n);
    }
    
    void close(Command::cmd_t cmd) {

        auto m = _mark.back();
        _mark.pop_back();
        
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

    auto x_var = axe::r_alpha() & axe::r_many(axe::r_alnum() | axe::r_lit('_'),0);

    auto y_mark = axe::e_ref([&](I b, I e) { stack.mark(); });
    auto y_mark_name = axe::e_ref([&](I b, I e) { stack.mark(std::string(b, e)); });
    auto y_close_arg = axe::e_ref([&](I b, I e) { stack.close(); });
    auto y_close_set = axe::e_ref([&](I b, I e) { stack.close(Command::SET); });
    auto y_close_arr = axe::e_ref([&](I b, I e) { stack.close(Command::ARR); });
    auto y_close_map = axe::e_ref([&](I b, I e) { stack.close(Command::MAP); });
    auto y_close_fun = axe::e_ref([&](I b, I e) { stack.close(Command::FUN); });
    
    auto x_from = ~((axe::r_lit(':') >> y_mark) & (x_expr >> y_close_arg));
    
    auto x_set =
        (axe::r_lit('{') >> y_mark) & (x_expr >> y_close_set) & x_from & axe::r_lit('}');
    
    auto x_array =
        (axe::r_lit('[') >> y_mark) & (x_expr >> y_close_arr) & x_from & axe::r_lit(']');
    
    auto x_map =
        (axe::r_lit('{')  >> y_mark) & (x_expr >> y_close_map) &
        (axe::r_lit("->") >> y_mark) & (x_expr >> y_close_arg) & x_from & axe::r_lit('}');
    
    auto x_funcall =
        (x_var >> y_mark_name) & x_ws & axe::r_lit('(') & (x_expr >> y_close_fun) & axe::r_lit(')');

    auto y_var_read = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, std::string(b, e)); });
    
    auto x_var_read = x_var >> y_var_read;

    auto x_expr_bottom =
        x_ws &
        (x_literal | x_funcall | x_var_read | x_set | x_array | x_map |
         (axe::r_lit('(') & x_expr_atom & axe::r_lit(')'))) &
        x_ws;

    auto y_expr_not = axe::e_ref([&](I b, I e) { stack.push(Command::NOT); });
    auto y_expr_neg = axe::e_ref([&](I b, I e) { stack.push(Command::NEG); });
    
    auto x_expr_neg =
        (axe::r_any("!") & x_expr_atom >> y_expr_not) |
        (axe::r_any("~") & x_expr_atom >> y_expr_neg) |
        x_expr_bottom;

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

    auto x_expr_assign =
        (x_ws & x_var & x_ws & axe::r_lit('=') & x_ws & x_expr_atom) |
        x_expr_atom;

    auto x_expr_seq = x_expr_assign & *(axe::r_lit(',') & x_expr_assign);

    x_expr = x_expr_seq;

    auto x_main = x_expr & axe::r_end();

    auto x_go = x_main |
        axe::r_fail([](I b, I e) {
                throw std::runtime_error("Syntax error, unparsed input: \"" + std::string(b, e) + "\"");
            });
    
    x_go(beg, end);

                            
    stack.print();
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
