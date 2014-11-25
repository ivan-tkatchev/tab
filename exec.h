#ifndef __TAB_EXEC_H
#define __TAB_EXEC_H

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
            //clo.object = obj::make(clo.type);
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
            
    obj::Object* o = rsub.stack.back();
    rsub.stack.clear();

    return o;
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

            obj::Sequencer& seq = obj::get<obj::Sequencer>(_exec_closure(rsub, c, 1));
            obj::Object* dst = c.object;
            
            while (1) {
                bool ok;
                
                obj::Object* next = seq.next(ok);

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

            obj::Sequencer& seq = obj::get<obj::Sequencer>(_exec_closure(rsub, c, 2));
            obj::Object* dst = c.object;
            
            while (1) {
                bool ok;
                
                obj::Object* next = seq.next(ok);

                rsub.set_toplevel(next);

                obj::Object* key = _exec_closure(rsub, c, 0);
                obj::Object* val = _exec_closure(rsub, c, 1);

                dst->map(key->clone(), val->clone());
                
                if (!ok) break;
            }

            r.stack.push_back(dst);

            break;
        }

        // And here comes the numeric operator boilerplate.

        case Command::EXP:
        {
            obj::Real& a = obj::get<obj::Real>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(r.stack.back());
            b.v = ::pow(b.v, a.v);
            break;
        }
        case Command::MUL_R:
        {
            obj::Real& a = obj::get<obj::Real>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(r.stack.back());
            b.v = b.v * a.v;
            break;
        }
        case Command::MUL_I:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v * a.v;
            break;
        }
        case Command::DIV_R:
        {
            obj::Real& a = obj::get<obj::Real>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(r.stack.back());
            b.v = b.v / a.v;
            break;
        }
        case Command::DIV_I:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v / a.v;
            break;
        }
        case Command::MOD:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v % a.v;
            break;
        }
        case Command::ADD_R:
        {
            obj::Real& a = obj::get<obj::Real>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(r.stack.back());
            b.v = b.v + a.v;
            break;
        }
        case Command::ADD_I:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v + a.v;
            break;
        }
        case Command::SUB_R:
        {
            obj::Real& a = obj::get<obj::Real>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(r.stack.back());
            b.v = b.v - a.v;
            break;
        }
        case Command::SUB_I:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v - a.v;
            break;
        }

        case Command::I2R_1:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(c.object);
            b.v = a.v;
            r.stack.push_back(c.object);
            break;
        }
        case Command::I2R_2:
        {
            obj::Object* x = r.stack.back();
            r.stack.pop_back();
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(c.object);
            b.v = a.v;
            r.stack.push_back(c.object);
            r.stack.push_back(x);
            break;
        }
        case Command::U2R_1:
        {
            obj::UInt& a = obj::get<obj::UInt>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(c.object);
            b.v = a.v;
            r.stack.push_back(c.object);
            break;
        }
        case Command::U2R_2:
        {
            obj::Object* x = r.stack.back();
            r.stack.pop_back();
            obj::UInt& a = obj::get<obj::UInt>(r.stack.back());
            r.stack.pop_back();
            obj::Real& b = obj::get<obj::Real>(c.object);
            b.v = a.v;
            r.stack.push_back(c.object);
            r.stack.push_back(x);
            break;
        }
        
        case Command::NOT:
        {
            obj::Int& o = obj::get<obj::Int>(r.stack.back());
            o.v = ~o.v;
            break;
        }
        case Command::AND:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v & a.v;
            break;
        }
        case Command::OR:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v | a.v;
            break;
        }
        case Command::XOR:
        {
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(r.stack.back());
            b.v = b.v ^ a.v;
            break;
        }
        }
    }
}

void execute(std::vector<Command>& commands, const Type& type, std::istream& inputs) {

    Runtime rt;

    obj::Object* toplevel = new obj::SequencerFile(inputs);
    rt.set_toplevel(toplevel);

    execute_init(commands);
    execute_run(commands, rt);

    obj::Object* res;
    
    if (rt.stack.size() != 1)
        throw std::runtime_error("Sanity error: did not produce result");
        
    res = rt.stack[0];
    res->print();
    std::cout << std::endl;
}

#endif
