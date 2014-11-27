#ifndef __TAB_INFER_H
#define __TAB_INFER_H

struct Functions {

    typedef void (*func_t)(const obj::Object*, obj::Object*&);

    typedef std::pair< String, Type > key_t;
    typedef std::pair< func_t, Type > val_t;
    
    std::unordered_map<key_t, val_t> funcs;

    typedef func_t (*checker_t)(const Type& args, Type& ret);
    
    std::unordered_map< String, checker_t > poly_funcs;
    
    Functions() {}

    void add(const std::string& name, const Type& args, const Type& out, func_t f) {

        String n = strings().add(name);
        funcs.insert(funcs.end(), std::make_pair(key_t(n, args), val_t(f, out)));
    }

    void add_poly(const std::string& name, checker_t c) {
        String n = strings().add(name);
        poly_funcs.insert(poly_funcs.end(), std::make_pair(n, c));
    }
                
    val_t get(const String& name, const Type& args) const {
            
        auto i = funcs.find(key_t(name, args));

        if (i != funcs.end())
            return i->second;

        auto j = poly_funcs.find(name);

        if (j != poly_funcs.end()) {

            Type ret;
            func_t f = (j->second)(args, ret);

            if (f != nullptr) 
                return val_t(f,ret);
        }

        throw std::runtime_error("Invalid function call: " + strings().get(name) + " " + Type::print(args));
    }
}; 

Functions& functions_init() {
    static Functions ret;
    return ret;
}

const Functions& functions() {
    return functions_init();
}

bool check_unsigned(const Type& t) {
    return (t.type == Type::ATOM && t.atom == Type::UINT);
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

std::vector<Command>::iterator
handle_real_operator(std::vector<Command>& commands, std::vector<Command>::iterator ci,
                     std::vector<Type>& stack, const std::string& name) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    bool r1 = check_real(t1);
    bool r2 = check_real(t2);

    if (r1 && r2) {
        // Nothing.

    } else {

        if (!r2) {
            ci = commands.insert(ci, Command(check_unsigned(t2) ? Command::U2R_2 : Command::I2R_2));
            ci->type = Type(Type::REAL);
            ++ci;
        }

        if (!r1) {
            ci = commands.insert(ci, Command(check_unsigned(t1) ? Command::U2R_1 : Command::I2R_1));
            ci->type = Type(Type::REAL);
            ++ci;
        }
    }

    stack.emplace_back(Type::REAL);

    return ci;
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

std::vector<Command>::iterator
handle_poly_operator(std::vector<Command>& commands, std::vector<Command>::iterator ci,
                     std::vector<Type>& stack, const std::string& name,
                     Command::cmd_t c_int, Command::cmd_t c_real, bool no_unsigned) {

    Type t1 = stack.back();
    stack.pop_back();
    Type t2 = stack.back();
    stack.pop_back();

    if (!check_numeric(t1) || !check_numeric(t2))
        throw std::runtime_error("Use of '" + name + "' operator on non-numeric value.");

    bool r1 = check_real(t1);
    bool r2 = check_real(t2);

    if (r1 || r2) {

        ci->cmd = c_real;
        
        if (!r1 || !r2) {

            Command::cmd_t newc;

            if (!r1 && check_unsigned(t1))
                newc = Command::U2R_1;
            else if (!r1)
                newc = Command::I2R_1;
            else if (check_unsigned(t2))
                newc = Command::U2R_2;
            else
                newc = Command::I2R_2;
        
            ci = commands.insert(ci, Command(newc));
            ci->type = Type(Type::REAL);
            ++ci;
        }

        stack.emplace_back(Type::REAL);

        return ci;
    }

    ci->cmd = c_int;

    if (!no_unsigned && check_unsigned(t1) && check_unsigned(t2)) {
        stack.emplace_back(Type::UINT);

    } else {
        stack.emplace_back(Type::INT);
    }

    return ci;
}

Type wrap_seq(const Type& t) {

    Type ret(Type::SEQ);

    if (t.type == Type::ARR || t.type == Type::SEQ) {

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
    std::unordered_map< String, std::pair<Type,UInt> > vars;

    void add_var(const String& name, const Type& type) {

        auto i = vars.find(name);

        if (i == vars.end()) {
            vars.insert(i, std::make_pair(name, std::make_pair(type, vars.size())));
            return;
        }

        i->second.first = type;
    }

    std::pair<Type,UInt> get_var(const String& name) const {

        auto i = vars.find(name);

        if (i == vars.end()) {
            throw std::runtime_error("Use of undefined variable: " + strings().get(name));
        }

        return i->second;
    }

    size_t num_vars() const {
        return vars.size();
    }
};


Type infer_types(std::vector<Command>& commands, const Type& toplevel, TypeRuntime& typer, bool allow_empty);

Type infer_closure(Command& c, size_t n, const Type& toplevel, TypeRuntime& typer,
                   const std::string& name, bool do_unwrap_seq = false, bool allow_empty = false) {

    if (n >= c.closure.size())
        throw std::runtime_error("Sanity error, asked to infer non-existing closure.");
    
    auto& cc = *(c.closure[n]);
    cc.type = infer_types(cc.code, toplevel, typer, allow_empty);

    if (do_unwrap_seq) 
        return unwrap_seq(cc.type);
    
    return cc.type;
}

Type infer_tup_generator(Command& c, Type toplevel, const TypeRuntime& _tr, const std::string& name, bool allow_empty = false) {

    if (c.closure.size() != 1)
        throw std::runtime_error("Sanity error, " + name + " is not a closure.");

    TypeRuntime typer;
    typer.vars = _tr.vars;

    return infer_closure(c, 0, toplevel, typer, name, false, allow_empty);
}

Type infer_gen_generator(Command& c, Type toplevel, const TypeRuntime& _tr, const std::string& name) {

    if (c.closure.size() != 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    TypeRuntime typer;
    typer.vars = _tr.vars;

    toplevel = infer_closure(c, 1, toplevel, typer, name, true);
        
    const Type& t = infer_closure(c, 0, toplevel, typer, name);

    Type ret(Type::SEQ);
    ret.push(t);

    return ret;
}

Type infer_arr_generator(const std::vector<Type>& stack) {

    const Type& t = stack.back();

    if (t.type != Type::SEQ)
        throw std::runtime_error("Sanity error, constructing array from a non-sequence.");

    Type t2 = unwrap_seq(t);

    if (t2.type == Type::SEQ)
        throw std::runtime_error("Cannot store sequences in array.");

    Type ret(Type::ARR);
    ret.push(t2);

    return ret;
}

Type infer_map_generator(const std::vector<Type>& stack) {

    const Type& t = stack.back();

    if (t.type != Type::SEQ)
        throw std::runtime_error("Sanity error, constructing map from a non-sequence.");

    Type t2 = unwrap_seq(t);

    if (t2.type != Type::TUP || !t2.tuple || t2.tuple->size() != 2)
        throw std::runtime_error("Sanity error, map constuction expects a sequence of pairs.");

    const Type& tk = t2.tuple->at(0);
    const Type& tv = t2.tuple->at(1);

    if (tk.type == Type::SEQ || tv.type == Type::SEQ)
        throw std::runtime_error("Cannot store sequences in a map");

    Type ret(Type::MAP);
    ret.push(tk);
    ret.push(tv);

    return ret;
}

Type infer_flat_generator(const std::vector<Type>& stack) {

    const Type& t = stack.back();

    if (t.type != Type::SEQ)
        throw std::runtime_error("Cannot flatten something that isn't a sequence.");

    Type t2 = unwrap_seq(t);
    t2 = wrap_seq(t2);

    return t2;
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


Type infer_types(std::vector<Command>& commands, const Type& toplevel, TypeRuntime& typer, bool allow_empty = false) {

    auto& stack = typer.stack;
    auto& vars = typer.vars;

    stack.clear();
    typer.add_var(strings().add("@"), toplevel);
    
    for (auto ci = commands.begin(); ci != commands.end(); ++ci) {
        Command& c = *ci;

        bool has_type = true;
        
        switch (c.cmd) {
        case Command::VAL:
            stack.emplace_back(c.arg);
            break;

        case Command::VAW:
            typer.add_var(c.arg.str, stack.back());
            stack.pop_back();
            has_type = false;
            break;
            
        case Command::VAR:
        {
            auto i = typer.get_var(c.arg.str);
            c.arg = Atom(i.second);
            stack.emplace_back(i.first);
            break;
        }
            
        case Command::EXP:
            ci = handle_real_operator(commands, ci, stack, "**");
            break;

        case Command::MUL_I:
        case Command::MUL_R:
            ci = handle_poly_operator(commands, ci, stack, "*", Command::MUL_I, Command::MUL_R, false);
            break;

        case Command::DIV_I:
        case Command::DIV_R:
            ci = handle_poly_operator(commands, ci, stack, "/", Command::DIV_I, Command::DIV_R, false);
            break;

        case Command::MOD:
            handle_int_operator(stack, "%");
            break;

        case Command::ADD_I:
        case Command::ADD_R:
            ci = handle_poly_operator(commands, ci, stack, "+", Command::ADD_I, Command::ADD_R, false);
            break;

        case Command::SUB_I:
        case Command::SUB_R:
            ci = handle_poly_operator(commands, ci, stack, "-", Command::SUB_I, Command::SUB_R, true);
            break;

        case Command::I2R_1:
        case Command::U2R_1:
        {
            Type t = stack.back();
            stack.pop_back();

            if (!check_integer(t))
                throw std::runtime_error("Casting a non-integer to real.");

            stack.emplace_back(Type::REAL);
            break;
        }

        case Command::I2R_2:
        case Command::U2R_2:
        {
            Type t1 = stack.back();
            stack.pop_back();
            Type t2 = stack.back();
            stack.pop_back();
            
            if (!check_integer(t2))
                throw std::runtime_error("Casting a non-integer to real.");

            stack.emplace_back(Type::REAL);
            stack.emplace_back(t1);
            break;
        }

        case Command::NOT:
        {
            Type t = stack.back();

            if (!check_integer(t)) 
                throw std::runtime_error("Use of '~' numeric operator on something other "
                                         "than integer or unsigned integer.");

            break;
        }

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
            Type t = infer_arr_generator(stack);
            stack.pop_back();
            stack.emplace_back(t);
            break;
        }

        case Command::MAP:
        {
            Type t = infer_map_generator(stack);
            stack.pop_back();
            stack.emplace_back(t);
            break;
        }

        case Command::GEN:
        {
            Type t = infer_gen_generator(c, toplevel, typer, "sequence generator");
            stack.emplace_back(t);
            break;
        }
        
        case Command::FUN:
        {
            Type args = infer_tup_generator(c, toplevel, typer, "function call", true);
            auto tmp = functions().get(c.arg.str, args);
            c.function = (void*)tmp.first;
            stack.emplace_back(tmp.second);
            break;
        }

        case Command::FLAT:
        {
            Type t = infer_flat_generator(stack);
            stack.pop_back();
            stack.emplace_back(t);
            break;
        }

        case Command::SEQ:
        {
            Type ti = stack.back();

            if (ti.type == Type::SEQ) {

                ci = commands.erase(ci);
                --ci;
                has_type = false;

            } else {
                
                Type to = wrap_seq(ti);
                stack.pop_back();
                stack.emplace_back(to);
            }
            break;
        }

        case Command::TUP:
        {
            if (stack.size() <= 1) {

                ci = commands.erase(ci);
                --ci;
                has_type = false;
                
            } else {
            
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

        if (has_type) {
            ci->type = stack.back();
        }
    }


    if (stack.size() == 0) {

        if (allow_empty) {
            return Type();
        } else {
            throw std::runtime_error("Empty sequences are not allowed.");
        }
    }

    if (stack.size() != 1)
        throw std::runtime_error("Sanity error: inferred multiple types.");
        
    return stack[0];
}

#endif
