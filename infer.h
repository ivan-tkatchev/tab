#ifndef __TAB_INFER_H
#define __TAB_INFER_H

struct Functions {

    typedef void (*func_t)(const obj::Object*, obj::Object*&);

    typedef std::pair< String, Type > key_t;
    typedef std::pair< func_t, Type > val_t;
    
    std::unordered_map<key_t, val_t> funcs;

    typedef func_t (*checker_t)(const Type& args, Type& ret, obj::Object*&);
    
    std::unordered_map< String, checker_t > poly_funcs;

    typedef obj::Object* (*seqmaker_t)(const Type& arg);

    seqmaker_t seqmaker;
    
    Functions() {}

    void add(const std::string& name, const Type& args, const Type& out, func_t f) {

        String n = strings().add(name);
        funcs.insert(funcs.end(), std::make_pair(key_t(n, args), val_t(f, out)));
    }

    void add_poly(const std::string& name, checker_t c) {
        String n = strings().add(name);
        poly_funcs.insert(poly_funcs.end(), std::make_pair(n, c));
    }

    void add_seqmaker(seqmaker_t sm) {
        seqmaker = sm;
    }

    val_t get(const String& name, const Type& args, obj::Object*& holder) const {
            
        auto i = funcs.find(key_t(name, args));

        if (i != funcs.end())
            return i->second;

        auto j = poly_funcs.find(name);

        if (j != poly_funcs.end()) {

            Type ret;
            func_t f = (j->second)(args, ret, holder);

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

    std::unordered_map< std::pair<String,size_t>, std::pair<Type,UInt> > vars;

    size_t nscopes;
    std::vector<size_t> scope;
    
    TypeRuntime() : nscopes(0) {
        scope.push_back(0);
    }

    UInt add_var(const String& name, const Type& type) {

        size_t sc = scope.back();

        auto i = vars.find(std::make_pair(name,sc));

        if (i == vars.end()) {

            UInt ret = vars.size();
            vars.insert(i, std::make_pair(std::make_pair(name, sc), std::make_pair(type, ret)));
            return ret;
        }

        i->second.first = type;
        return i->second.second;
    }

    std::pair<Type,UInt> get_var(const String& name) const {

        for (auto si = scope.rbegin(); si != scope.rend(); ++si) {
        
            auto i = vars.find(std::make_pair(name,*si));

            if (i != vars.end())
                return i->second;
        }
            
        throw std::runtime_error("Use of undefined variable: " + strings().get(name));
    }

    size_t num_vars() const {
        return vars.size();
    }

    void enter_scope() {
        nscopes++;
        scope.push_back(nscopes);
    }

    void exit_scope() {

        if (scope.size() <= 1)
            throw std::runtime_error("Sanity error: exited toplevel scope.");

        scope.pop_back();
    }
};


Type infer_expr(std::vector<Command>& commands, TypeRuntime& typer, bool allow_empty);

Type infer_gen_generator(Command& c, TypeRuntime& typer, UInt& tlvar) {

    if (c.closure.size() != 2)
        throw std::runtime_error("Sanity error, generator is not a closure.");

    typer.enter_scope();

    Command::Closure& clo0 = *(c.closure[0]);
    Command::Closure& clo1 = *(c.closure[1]);

    Type toplevel = unwrap_seq(infer_expr(clo1.code, typer, false));

    tlvar = typer.add_var(strings().add("@"), toplevel);
    
    Type t = infer_expr(clo0.code, typer, false);
    
    Type ret(Type::SEQ);
    ret.push(t);

    typer.exit_scope();
    
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

Type infer_expr(std::vector<Command>& commands, TypeRuntime& typer, bool allow_empty = false) {

    std::vector<Type> stack;
    auto& vars = typer.vars;

    for (auto ci = commands.begin(); ci != commands.end(); ++ci) {
        Command& c = *ci;

        bool has_type = true;
        
        switch (c.cmd) {
        case Command::VAL:
            stack.emplace_back(c.arg);
            stack.back().literal = std::make_shared<Atom>(c.arg);
            break;

        case Command::VAW:
        {
            UInt i = typer.add_var(c.arg.str, stack.back());
            c.arg = Atom(i);
            stack.pop_back();
            has_type = false;
            break;
        }

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

        case Command::EQ:
        case Command::NEQ:
        {
            Type t1 = stack.back();
            stack.pop_back();
            Type t2 = stack.back();
            stack.pop_back();

            if (check_numeric(t1) && check_numeric(t2)) {
                ci = handle_poly_operator(commands, ci, stack, "==", c.cmd, c.cmd, false);

                // HACK Undo unneeded actions of 'handle_poly_operator'.
                stack.pop_back();

            } else if (t1 != t2) {
                throw std::runtime_error("Only objects of the same type can be compared.");
            }

            stack.emplace_back(Type::UINT);
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
            UInt tlvar;
            Type t = infer_gen_generator(c, typer, tlvar);
            c.arg.uint = tlvar;
            stack.emplace_back(t);

            Command::Closure& clo1 = *(c.closure[1]);
            ci = commands.insert(ci, clo1.code.begin(), clo1.code.end());
            ci += clo1.code.size();
            ci->closure.pop_back();
            
            break;
        }
        
        case Command::FUN:
        case Command::FUN0:
        {

            if (c.closure.size() != 1)
                throw std::runtime_error("Sanity error, function call without arguments.");

            Command::Closure& clo = *(c.closure[0]);

            Type args = infer_expr(clo.code, typer, true);

            auto tmp = functions().get(c.arg.str, args, c.object);
            c.function = (void*)tmp.first;
            stack.emplace_back(tmp.second);

            if (args.type == Type::NONE)
                c.cmd = Command::FUN0;
            else
                c.cmd = Command::FUN;
            
            ci = commands.insert(ci, clo.code.begin(), clo.code.end());
            ci += clo.code.size();
            ci->closure.clear();

            break;
        }

        case Command::SEQ:
        {
            Type ti = stack.back();

            // HACK!
            c.object = (functions().seqmaker)(ti);

            if (c.object == nullptr) {

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
                c.arg = Atom(UInt(t.tuple->size()));
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
            throw std::runtime_error("Expression computes nothing.");
        }
    }

    if (stack.size() != 1)
        throw std::runtime_error("Sanity error: inferred multiple types.");
        
    return stack.back();
}

Type infer(std::vector<Command>& commands, const Type& toplevel, TypeRuntime& typer) {

    typer.add_var(strings().add("@"), toplevel);
    return infer_expr(commands, typer);
}

#endif
