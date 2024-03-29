#ifndef __TAB_PARSE_H
#define __TAB_PARSE_H

#include "axe.h"

namespace tab {

struct ParseStack {

    std::vector<Command> stack;

    struct mark_val_t {
        size_t top;
        bool try_catch;
        String name;

        mark_val_t(size_t t = 0, bool tc = false, String n = String{0}) :
            top(t), try_catch(tc), name(n) {}
    };

    std::vector<mark_val_t> _mark;
    std::vector<String> names;
    std::vector<UInt> counters;
    std::vector<UInt> expr_len;
    std::vector<size_t> rollback_mark;

    std::vector<std::string> errors;

    void push(Command::cmd_t c) { stack.emplace_back(c); }

    template <typename T>
    void push(Command::cmd_t c, const T& t) { stack.emplace_back(c, t); }

    void mark(bool tc = false, String n = String{0}) {
        _mark.emplace_back(stack.size(), tc, n);
    }

    void unmark() {
        if (_mark.empty()) {
            throw std::runtime_error("unmark() sanity error when parsing");
        }
        _mark.pop_back();
    }

    void mark_rollback() {
        rollback_mark.push_back(stack.size());
    }

    void unmark_rollback() {
        if (rollback_mark.empty()) {
            throw std::runtime_error("unmark_rollback() sanity error when parsing");
        }
        rollback_mark.pop_back();
    }

    void rollback() {
        if (rollback_mark.empty()) {
            throw std::runtime_error("rollback() sanity error when parsing");
        }
        stack.resize(rollback_mark.back());
        rollback_mark.pop_back();
    }

    mark_val_t close_to(std::vector<Command>& otherstack, bool do_pop = true) {
        if (_mark.empty()) {
            throw std::runtime_error("close_to() sanity error when parsing");
        }

        push(Command::TUP);

        auto ret = _mark.back();

        if (do_pop)
            _mark.pop_back();

        otherstack.assign(stack.begin() + ret.top, stack.end());
        stack.erase(stack.begin() + ret.top, stack.end());

        return ret;
    }

    void close(Command::cmd_t cmd, bool do_pop = true, Command::cmd_t altcmd = Command::cmd_t{}) {
        if (_mark.empty()) {
            throw std::runtime_error("close() sanity error when parsing");
        }

        Command::Closure c;

        auto mark_val = close_to(c.code, do_pop);

        cmd = (mark_val.try_catch ? altcmd : cmd);

        if (mark_val.name.ix == 0) {
            stack.emplace_back(cmd);
        } else {
            stack.emplace_back(cmd, mark_val.name);
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
                i.cmd == Command::FUN0 || i.cmd == Command::TUP || i.cmd == Command::LAMD ||
                (print_types && (i.cmd == Command::GEN || i.cmd == Command::GEN_TRY || i.cmd == Command::REC))) {

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
Type parse(I beg, I end, const Type& toplevel_type, TypeRuntime& typer, std::vector<Command>& commands, unsigned int debuglevel = 0) {

    ParseStack stack;
    std::string str_buff;
    
    axe::r_rule<I> x_expr;
    axe::r_rule<I> x_expr_atom;
    axe::r_rule<I> x_expr_bit;
    axe::r_rule<I> x_expr_bottom;
    
    auto x_ws = *(axe::r_any(" \t\n") | (axe::r_lit('#') & axe::r_many(axe::r_any() - axe::r_lit('\n'), 0)));

    auto y_int = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stol(std::string(b, e)));
            } catch (std::exception& ex) {
                stack.errors.emplace_back("Could not convert '" + std::string(b, e) + "' to an integer.");
            }
        });

    auto x_int = ((axe::r_lit('-') & +axe::r_num() & ~axe::r_any("sli")) |
                  (+axe::r_num() & axe::r_any("sli")))
        >> y_int;

    auto y_uint = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stoul(std::string(b, e), nullptr, 0));
            } catch (std::exception& ex) {
                stack.errors.emplace_back("Could not convert '" + std::string(b, e) + "' to an unsigned integer.");
            }
        });
    
    auto x_uint = (+axe::r_num() & ~axe::r_lit('u'))
        >> y_uint;

    auto x_hex = ((axe::r_lit("0x") | axe::r_lit("0X")) & +axe::r_hex())
        >> y_uint;

    auto y_float = axe::e_ref([&](I b, I e) {
            try {
                stack.push(Command::VAL, std::stod(std::string(b, e)));
            } catch (std::exception& ex) {
                stack.errors.emplace_back("Could not convert '" + std::string(b, e) + "' to a floating-point number.");
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

    auto y_close_fun = axe::e_ref([&](I b, I e) { stack.close(Command::FUN); });
    auto y_unmark = axe::e_ref([&](I b, I e) { stack.unmark(); });

    auto y_mark_strinterp = axe::e_ref([&](I b, I e) {
        stack.mark(false, make_string("string_interpolate"));
        str_buff.clear(); });

    auto y_strinterp_end = axe::e_ref([&](I b, I e) {
        if (!str_buff.empty()) {
            stack.push(Command::VAL, strings().add(str_buff));
        }
        stack.mark_rollback();
    });

    //auto y_mark_rollback = axe::e_ref([&](I b, I e) { stack.mark_rollback(); });
    auto y_unmark_rollback = axe::e_ref([&](I b, I e) { stack.unmark_rollback(); });
    auto y_rollback = axe::e_ref([&](I b, I e) { stack.rollback(); str_buff.clear(); });

    auto x_char3 =
        x_quotedchar |
        (axe::r_lit("${") >> y_strinterp_end & ((x_expr & axe::r_lit("}")) >> y_unmark_rollback | r_fail(y_rollback))) >> y_string_start |
        (axe::r_any() - axe::r_lit('\\') - axe::r_lit('`')) >> y_char;

    auto x_strinterp = 
        (axe::r_lit('`') >> y_mark_strinterp) & axe::r_many(x_char3,0) & (axe::r_lit('`') >> y_string_end >> y_close_fun);

    auto x_literal = x_hex | x_float | x_int | x_uint | x_string | x_strinterp;

    auto x_ident = (axe::r_alpha() & axe::r_many(axe::r_alnum() | axe::r_lit('_'),0));
    auto x_var = axe::r_lit('@') | x_ident;

    auto y_mark = axe::e_ref([&](I b, I e) { stack.mark(); });
    auto y_mark_try = axe::e_ref([&](I b, I e) { stack.mark(true); });
    auto y_mark_name = axe::e_ref([&](I b, I e) { stack.mark(false, make_string(b, e)); });

    auto y_close_seq = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); stack.push(Command::SEQ); });
    auto y_close_arg = axe::e_ref([&](I b, I e) { stack.close(); });
    auto y_close_gen = axe::e_ref([&](I b, I e) { stack.close(Command::GEN, true, Command::GEN_TRY); });
    auto y_close_rec = axe::e_ref([&](I b, I e) { stack.close(Command::REC); });
    
    auto y_true = axe::e_ref([&](I b, I e) { stack.push(Command::VAL, (Int)1); });

    auto y_default_from = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, strings().add("@")); });
    
    auto x_from =
        (((axe::r_lit(':') >> y_mark) & x_expr) |
         (axe::r_empty() >> y_mark >> y_default_from)) >> y_close_seq >> y_close_arg;

    auto x_opt_try_mark = ((x_ws & axe::r_lit("try") >> y_mark_try) | (axe::r_empty() >> y_mark));

    auto y_mark_if = axe::e_ref([&](I b, I e) { stack.mark(false, make_string("if")); });

    auto x_filter_generator = 
        ((axe::r_lit('[') & x_ws & axe::r_lit('/') & x_ws) >> y_mark_try >> y_mark_if) &
        (x_expr_atom >> y_default_from >> y_close_fun >> y_close_gen) & x_from & axe::r_lit(']');

    auto x_generator =
        x_filter_generator | 
        ((axe::r_lit('[') & x_opt_try_mark) & (x_expr >> y_close_gen) & x_from & axe::r_lit(']'));

    auto y_array = axe::e_ref([&](I b, I e) { stack.push(Command::ARR); });
    auto y_map = axe::e_ref([&](I b, I e) { stack.push(Command::MAP); });
    auto y_close_tup = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); });
    auto y_close_tup1 = axe::e_ref([&](I b, I e) { stack.push(Command::TUP, UInt(1)); });

    auto x_array =
        (axe::r_lit("[.") & x_opt_try_mark) & (x_expr >> y_close_gen) & x_from & axe::r_lit(".]") >> y_array;

    auto x_map =
        (axe::r_lit('{') & x_opt_try_mark) & (x_expr >> y_close_tup) &
        (((axe::r_lit("->") & (x_expr >> y_close_tup1)) |
          (axe::r_empty() >> y_true)) >> y_close_tup >> y_close_gen) &
        x_from & axe::r_lit('}') >> y_map;

    auto x_recursor =
        (axe::r_lit("<<") >> y_mark) & (x_expr >> y_close_rec) &
        (((axe::r_lit(':') >> y_mark) & x_expr) >> y_close_arg) & axe::r_lit(">>");

    auto x_funcall_b =
        (x_ident >> y_mark_name) &
        ((x_ws & axe::r_lit('(') & ~x_expr & x_ws & (axe::r_lit(')') >> y_close_fun)) |
         r_fail(y_unmark));

    auto x_funcall_d =
        (x_ident >> y_mark_name) &
        ((x_ws & axe::r_lit('.') & (x_expr_bit >> y_close_fun)) |
         r_fail(y_unmark));

    auto y_mark_dollar = axe::e_ref([&](I b, I e) { stack.mark(false, make_string("$")); });

    auto x_funcall_dollar =
        (axe::r_lit('$') >> y_mark_dollar >> y_default_from) & 
        (((x_ws & axe::r_lit('(') & x_expr & x_ws & axe::r_lit(')')) |
          x_expr_bottom)
         >> y_close_fun);

    auto x_funcall = x_funcall_b | x_funcall_d | x_funcall_dollar;

    auto y_var_read = axe::e_ref([&](I b, I e) { stack.push(Command::VAR, make_string(b, e)); });
    
    auto x_var_read = x_var >> y_var_read;

    x_expr_bottom =
        x_ws &
        (x_literal | x_funcall | x_var_read | x_array | x_map | x_generator | x_recursor |
         (axe::r_lit('(') & x_expr_atom & axe::r_lit(')'))) &
        x_ws;

    auto y_mark_idx = axe::e_ref([&](I b, I e) { stack.mark(false, make_string("index")); });
    auto y_close_idx = axe::e_ref([&](I b, I e) { stack.close(Command::FUN, false); });

    auto x_index_brac = axe::r_lit('[') & x_expr & axe::r_lit(']') & x_ws >> y_close_idx;
    auto x_index_tild = axe::r_lit('~') & x_expr_bottom >> y_close_idx;
    auto x_index = *(x_index_brac | x_index_tild);
    
    auto x_expr_idx =
        (axe::r_empty() >> y_mark_idx) &
        ((x_expr_bottom & (x_index >> y_unmark)) |
         r_fail(y_unmark));

    auto y_mark_flat = axe::e_ref([&](I b, I e) { stack.mark(false, make_string("flatten")); });
    auto y_mark_filter = axe::e_ref([&](I b, I e) { stack.mark(false, make_string("filter")); });

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

    x_expr_bit =
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

    auto x_expr_andor =
        x_expr_eq & *((axe::r_lit("&&") & x_expr_eq) >> y_expr_and |
                       (axe::r_lit("||") & x_expr_eq) >> y_expr_or);

    auto y_expr_pipe = axe::e_ref([&](I b, I e) { stack.push(Command::VAW, strings().add("@")); });

    auto x_expr_pipe =
	x_expr_andor & *((axe::r_lit("..") >> y_expr_pipe) & x_expr_andor);

    x_expr_atom = x_expr_pipe;

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

    auto x_expr_defbody =
        (axe::r_lit('(') & x_expr & axe::r_lit(')')) |
        (x_expr_atom);

    auto x_expr_defname = 
        ((x_ident | axe::r_lit('$')) >> y_mark_name) &
        x_ws &
        ((x_expr_defbody >> y_expr_define) | r_fail(y_unmark));

    auto y_start_nth = axe::e_ref([&](I b, I e) { stack.counters.push_back(0); });
    auto y_end_nth = axe::e_ref([&](I b, I e) { stack.counters.pop_back(); });
    auto y_index_nth = axe::e_ref([&](I b, I e) {
            stack.push(Command::VAL, stack.counters.back());
            stack.counters.back()++;
            stack.close(Command::FUN);
            stack.push(Command::VAW, strings().add("@"));
        });

    auto y_undo = axe::e_ref([&](I b, I e) { stack.stack.pop_back(); });

    auto x_expr_defnth =
        (x_ws & (x_ident >> y_mark_name >> y_mark_idx >> y_default_from >> y_index_nth) &
         x_ws & (x_expr_atom | (axe::r_empty() >> y_undo)) >> y_expr_define);

    auto x_expr_defstruct =
        (axe::r_lit('[') >> y_start_nth) &
        ((x_expr_defnth & +(axe::r_lit(',') & x_expr_defnth) &
          (axe::r_lit(']') >> y_end_nth))
         | r_fail(y_start_nth));

    auto x_expr_define =
        x_ws &
        axe::r_lit("def") & x_ws & (x_expr_defname | x_expr_defstruct);

    auto y_start_expr = axe::e_ref([&](I b, I e) { stack.expr_len.emplace_back(0); });
    auto y_incr_expr_len = axe::e_ref([&](I b, I e) { stack.expr_len.back()++; });
    auto y_end_expr =
	axe::e_ref([&](I b, I e) {
		       UInt len = stack.expr_len.back();

		       if (len == 0) {
                           stack.errors.emplace_back("Expression '" + std::string(b, e) + "' has no value. (Assignment to a variable is not a value.)");
		       }

		       stack.expr_len.pop_back();
		   });

    auto x_topexpr =
        x_expr_define |
        x_expr_assign |
        (x_expr_atom >> y_incr_expr_len);

    auto x_expr_seq = (axe::r_empty() >> y_start_expr) & (x_topexpr & *(axe::r_any(",;") & x_topexpr)) >> y_end_expr;

    x_expr = x_expr_seq;

    auto y_end = axe::e_ref([&](I b, I e) { stack.push(Command::TUP); });
    
    auto x_main = x_expr & axe::r_end() >> y_end;

    auto x_go = x_main |
        axe::r_fail([](I b, I e) {
                throw std::runtime_error("Syntax error, unparsed input: \"" + std::string(b, e) + "\"");
            });

    x_go(beg, end);

    if (stack.errors.size() > 0) {

        std::string msg;
        for (const std::string& m : stack.errors) {
            msg += m;
            msg += "\n";
        }
        msg.pop_back();

        throw std::runtime_error(msg);
    }

    if (debuglevel >= 3) {
        std::cout << "[Parse tree]" << std::endl;
        stack.print(false);
        std::cout << std::endl;
    }

    Type ret = infer(stack.stack, toplevel_type, typer);

    optimize(stack.stack, typer);

    if (debuglevel >= 2) {
        std::cout << "\n[Program]" << std::endl;
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

} // namespace tab

#endif
