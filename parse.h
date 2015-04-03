#ifndef __TAB_PARSE_H
#define __TAB_PARSE_H

#include "axe.h"

struct ParseStack {

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

    String close_to(std::vector<Command>& otherstack, bool do_pop = true) {

        push(Command::TUP);

        auto m = _mark.back();
        String ret = m.second;

        if (do_pop)
            _mark.pop_back();

        otherstack.assign(stack.begin() + m.first, stack.end());
        stack.erase(stack.begin() + m.first, stack.end());

        return ret;
    }

    void close(Command::cmd_t cmd, bool do_pop = true) {

        Command::Closure c;

        String name = close_to(c.code, do_pop);

        if (name.ix == 0) {
            stack.emplace_back(cmd);
        } else {
            stack.emplace_back(cmd, name);
        }
        
        stack.back().closure.resize(1);
        stack.back().closure[0].swap(c);
    }

    void close() {

        Command::Closure c;

        close_to(c.code);

        stack.back().closure.resize(stack.back().closure.size() + 1);
        stack.back().closure.back().swap(c);
    }
    
    static void print(const std::vector<Command>& c, size_t level, bool print_types) {

        for (const auto& i : c) {
            std::cout << " " << std::string(level*2, ' ') << Command::print(i.cmd);

            if (i.cmd == Command::VAL || i.cmd == Command::VAR || i.cmd == Command::VAW || i.cmd == Command::FUN ||
                i.cmd == Command::TUP || i.cmd == Command::LAMD ||
                (print_types && i.cmd == Command::GEN)) {

                std::cout << " " << Atom::print(i.arg);
            }

            if (print_types) 
                std::cout << " --> " << Type::print(i.type);

            std::cout << std::endl;

            bool first = true;
            for (const auto& ii : i.closure) {

                if (first) {
                    first = false;
                } else {
                    std::cout << " " << std::string(level*2, ' ') << "-" << std::endl;
                }

                print(ii.code, level + 1, print_types);
            }
        }
    }        
    
    void print(bool print_types = true) const {
        print(stack, 0, print_types);
    }
};

template <typename I>
String make_string(I beg, I end) {
    return strings().add(std::string(beg, end));
}

String make_string(const std::string& s) {
    return strings().add(s);
}

template <typename I>
Type parse(I beg, I end, TypeRuntime& typer, std::vector<Command>& commands, unsigned int debuglevel = 0) {

    ParseStack stack;
    std::string str_buff;
    
    axe::r_rule<I> x_expr;
    axe::r_rule<I> x_expr_atom;
    
    auto x_ws = *axe::r_any(" \t\n");

    auto y_int = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stol(std::string(b, e)));
            } catch (std::exception& ex) {
                throw std::runtime_error("Could not convert '" + std::string(b, e) + "' to an integer.");
            }
        });
    
    auto x_int = (~axe::r_lit('-') & +axe::r_num())
        >> y_int;

    auto y_uint = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stoul(std::string(b, e)));
            } catch (std::exception& ex) {
                throw std::runtime_error("Could not convert '" + std::string(b, e) + "' to an integer.");
            }
        });
    
    auto x_uint = (+axe::r_num() & axe::r_lit('u'))
        >> y_uint;

    auto y_float = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stod(std::string(b, e)));
            } catch (std::exception& ex) {
                throw std::runtime_error("Could not convert '" + std::string(b, e) + "' to a floating-point number.");
            }
        });

    auto x_floathead = ~axe::r_any("-+") & +axe::r_num();
    auto x_floatdots = axe::r_lit('.') & axe::r_many(axe::r_num(),0);
    auto x_floatexp = x_floathead & ~x_floatdots & axe::r_any("eE") & x_floathead;
    auto x_floatdot = x_floathead & x_floatdots;
    auto x_floatlit = x_floatexp | x_floatdot;

    auto x_float = x_floatlit >> y_float;

    auto y_string_start = axe::e_ref([&](I b, I e) { str_buff.clear(); });
    auto y_string_end = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, strings().add(str_buff)); });
    auto y_quotedchar = axe::e_ref([&](I b, I e) {

            std::string z(b, e);

            if (z == "t") {
                str_buff += '\t';
            } else if (z == "n") {
                str_buff += '\n';
            } else if (z == "r") {
                str_buff += '\r';
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

    auto x_ident = (axe::r_alpha() & axe::r_many(axe::r_alnum() | axe::r_lit('_'),0));
    auto x_var = axe::r_lit('@') | x_ident;

    auto y_mark = axe::e_ref([&](I b, I e) { stack.mark(); });
    auto y_mark_name = axe::e_ref([&](I b, I e) { stack.mark(make_string(b, e)); });
    auto y_unmark = axe::e_ref([&](I b, I e) { stack.unmark(); });

    auto y_close_seq = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); stack.push(Command::SEQ); });
    auto y_close_arg = axe::e_ref([&](I b, I e) { stack.close(); });
    auto y_close_gen = axe::e_ref([&](I b, I e) { stack.close(Command::GEN); });
    
    auto y_true = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, (Int)1); });

    auto y_default_from = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, strings().add("@")); });
    
    auto x_from =
        (((axe::r_lit(':') >> y_mark) & x_expr) |
         (axe::r_empty() >> y_mark >> y_default_from)) >> y_close_seq >> y_close_arg;

    auto x_generator =
        (axe::r_lit('[') >> y_mark) & (x_expr >> y_close_gen) & x_from & axe::r_lit(']');

    auto y_array = axe::e_ref([&](I b, I e) { stack.push(Command::ARR); });
    auto y_map = axe::e_ref([&](I b, I e) { stack.push(Command::MAP); });
    auto y_close_tup = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); });
    auto y_close_tup1 = axe::e_ref([&](I b, I e) { stack.push(Command::TUP, UInt(1)); });

    auto x_array =
        (axe::r_lit("[.") >> y_mark) & (x_expr >> y_close_gen) & x_from & axe::r_lit(".]") >> y_array;

    auto x_map =
        (axe::r_lit('{')  >> y_mark) & (x_expr >> y_close_tup) &
        (((axe::r_lit("->") & (x_expr >> y_close_tup1)) |
          (axe::r_empty() >> y_true)) >> y_close_tup >> y_close_gen) &
        x_from & axe::r_lit('}') >> y_map;

    auto y_close_fun = axe::e_ref([&](I b, I e) { stack.close(Command::FUN); });

    auto x_funcall_b =
        (x_ident >> y_mark_name) &
        ((x_ws & axe::r_lit('(') & ~x_expr & x_ws & (axe::r_lit(')') >> y_close_fun)) |
         r_fail(y_unmark));

    auto x_funcall_d =
        (x_ident >> y_mark_name) &
        ((x_ws & axe::r_lit('.') & (x_expr_atom >> y_close_fun)) |
         r_fail(y_unmark));

    auto x_funcall = x_funcall_b | x_funcall_d;

    auto y_var_read = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, make_string(b, e)); });
    
    auto x_var_read = x_var >> y_var_read;

    auto x_expr_bottom =
        x_ws &
        (x_literal | x_funcall | x_var_read | x_array | x_map | x_generator | 
         (axe::r_lit('(') & x_expr_atom & axe::r_lit(')'))) &
        x_ws;

    auto y_mark_idx = axe::e_ref([&](I b, I e) { stack.mark(make_string("index")); });
    auto y_close_idx = axe::e_ref([&](I b, I e) { stack.close(Command::FUN, false); });

    auto x_index_brac = axe::r_lit('[') & x_expr & axe::r_lit(']') & x_ws >> y_close_idx;
    auto x_index_tild = axe::r_lit('~') & x_expr_bottom >> y_close_idx;
    auto x_index = *(x_index_brac | x_index_tild);
    
    auto x_expr_idx =
        (axe::r_empty() >> y_mark_idx) &
        ((x_expr_bottom & (x_index >> y_unmark)) |
         r_fail(y_unmark));

    auto y_mark_flat = axe::e_ref([&](I b, I e) { stack.mark(make_string("flatten")); });
    auto y_mark_filter = axe::e_ref([&](I b, I e) { stack.mark(make_string("filter")); });
    
    axe::r_rule<I> x_expr_flat;
    x_expr_flat =
        (axe::r_lit(':') >> y_mark_flat & x_expr_flat >> y_close_fun) |
        (axe::r_lit('?') >> y_mark_filter & x_expr_flat >> y_close_fun) |
        x_expr_idx;
    
    auto y_expr_not = axe::e_ref([&](I b, I e) { stack.push(Command::NOT); });

    axe::r_rule<I> x_expr_not;
    x_expr_not = x_ws &
        ((axe::r_lit('!') & x_expr_not >> y_expr_not) |
         x_expr_flat);

    auto y_expr_exp = axe::e_ref([&](I b, I e) { stack.push(Command::EXP); });

    auto x_expr_exp =
        x_expr_not & *(axe::r_lit("**") & x_expr_not >> y_expr_exp);

    auto y_expr_mul = axe::e_ref([&](I b, I e) { stack.push(Command::MUL_R); });
    auto y_expr_div = axe::e_ref([&](I b, I e) { stack.push(Command::DIV_R); });
    auto y_expr_mod = axe::e_ref([&](I b, I e) { stack.push(Command::MOD); });

    auto x_expr_mul =
        x_expr_exp & *((axe::r_lit('*') & x_expr_exp) >> y_expr_mul |
                       (axe::r_lit('/') & x_expr_exp) >> y_expr_div |
                       (axe::r_lit('%') & x_expr_exp) >> y_expr_mod);

    auto y_expr_add = axe::e_ref([&](I b, I e) { stack.push(Command::ADD_R); });
    auto y_expr_sub = axe::e_ref([&](I b, I e) { stack.push(Command::SUB_R); });

    auto x_expr_add =
        x_expr_mul & *((axe::r_lit('+') & x_expr_mul) >> y_expr_add |
                       (axe::r_lit('-') & x_expr_mul) >> y_expr_sub);

    auto y_expr_and = axe::e_ref([&](I b, I e) { stack.push(Command::AND); });
    auto y_expr_or  = axe::e_ref([&](I b, I e) { stack.push(Command::OR); });
    auto y_expr_xor = axe::e_ref([&](I b, I e) { stack.push(Command::XOR); });

    auto x_expr_bit =
        x_expr_add & *((axe::r_lit('&') & x_expr_add) >> y_expr_and |
                       (axe::r_lit('|') & x_expr_add) >> y_expr_or |
                       (axe::r_lit('^') & x_expr_add) >> y_expr_xor);

    auto y_expr_eq  = axe::e_ref([&](I b, I e) { stack.push(Command::EQ); });
    auto y_expr_neq = axe::e_ref([&](I b, I e) { stack.push(Command::EQ); stack.push(Command::NEG); });
    auto y_expr_lt = axe::e_ref([&](I b, I e) { stack.push(Command::LT); });
    auto y_expr_gt = axe::e_ref([&](I b, I e) { stack.push(Command::ROT); stack.push(Command::LT); });
    auto y_expr_lte = axe::e_ref([&](I b, I e) { stack.push(Command::ROT); stack.push(Command::LT);
                                                 stack.push(Command::NEG); });
    auto y_expr_gte = axe::e_ref([&](I b, I e) { stack.push(Command::LT); stack.push(Command::NEG); });

    auto x_expr_eq =
        x_expr_bit & ~((axe::r_lit("==") & x_expr_bit) >> y_expr_eq |
                       (axe::r_lit("!=") & x_expr_bit) >> y_expr_neq |
                       (axe::r_lit("<") & x_expr_bit) >> y_expr_lt |
                       (axe::r_lit(">") & x_expr_bit) >> y_expr_gt |
                       (axe::r_lit("<=") & x_expr_bit) >> y_expr_lte |
                       (axe::r_lit(">=") & x_expr_bit) >> y_expr_gte);

    x_expr_atom = x_expr_eq;

    auto y_expr_assign_var = axe::e_ref([&](I b, I e) { stack.names.emplace_back(make_string(b, e)); });
    auto y_expr_assign = axe::e_ref([&](I b, I e) { stack.push(Command::VAW, stack.names.back());
                                                    stack.names.pop_back(); });

    auto y_no_assign = axe::e_ref([&](I b, I e) { stack.names.pop_back(); });

    auto y_expr_define = axe::e_ref([&](I b, I e) { stack.close(Command::LAMD); });
    
    auto x_expr_assign = 
        x_ws &
        (x_var >> y_expr_assign_var) &
        ((x_ws & axe::r_lit('=') & x_ws & (x_expr_atom >> y_expr_assign))
         | r_fail(y_no_assign));

    auto x_expr_define =
        x_ws &
        axe::r_lit("def") & x_ws &
        (x_ident >> y_mark_name) &
        x_ws &
        (x_expr_atom >> y_expr_define);

    auto x_topexpr =
        x_expr_define |
        x_expr_assign |
        x_expr_atom;

    auto x_expr_seq = x_topexpr & *(axe::r_lit(',') & x_topexpr);

    x_expr = x_expr_seq;

    auto y_end = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); });
    
    auto x_main = x_expr & axe::r_end() >> y_end;

    auto x_go = x_main |
        axe::r_fail([](I b, I e) {
                throw std::runtime_error("Syntax error, unparsed input: \"" + std::string(b, e) + "\"");
            });
    
    x_go(beg, end);

    if (debuglevel >= 3) {
        std::cout << "[Parse tree]" << std::endl;
        stack.print(false);
        std::cout << std::endl;
    }
    
    Type toplevel(Type::SEQ);
    toplevel.push(Type::STRING);
    
    Type ret = infer(stack.stack, toplevel, typer);

    if (debuglevel >= 2) {
        std::cout << "[Program]" << std::endl;    
        stack.print();
        std::cout << std::endl;
    }

    if (debuglevel >= 1) {
        std::cout << "--> " << Type::print(ret) << std::endl;
        std::cout << std::endl;
    }

    commands.swap(stack.stack);

    return ret;
}

#endif
