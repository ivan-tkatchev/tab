#ifndef __TAB_EXEC_H
#define __TAB_EXEC_H

struct Runtime {
    std::vector<obj::Object*> vars;
    std::vector<obj::Object*> stack;

    Runtime(size_t nvars) {
        vars.resize(nvars);
    }
    
    void set_var(UInt ix, obj::Object* o) {
        std::cout << "SETTING: " << ix << " ";
        o->print();
        std::cout << std::endl;
        vars[ix] = o;
    }

    obj::Object* get_var(UInt ix) {
        std::cout << "GETTING: " << ix << " ";
        vars[ix]->print();
        std::cout << std::endl;
        return vars[ix];
    }
};


void execute_init(std::vector<Command>& commands) {

    for (auto& c : commands) {

        for (auto& cloptr : c.closure) {
            auto& clo = *cloptr;
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

        case Command::FLAT:
            c.object = new obj::SequencerFlatten(c.type);
            break;

        case Command::FUN:
            if (c.object == nullptr)
                c.object = obj::make(c.type);
            break;
            
        default:
            c.object = obj::make(c.type);
            break;
        }
    }
}

void execute_run(std::vector<Command>& commands, Runtime& r, size_t mark);

obj::Object* _exec_closure(Runtime& r, Command::Closure& closure) {

    size_t mark = r.stack.size();

    execute_run(closure.code, r, mark);

    obj::Object* o = r.stack.back();
    r.stack.pop_back();

    return o;
}


void execute_run(std::vector<Command>& commands, Runtime& r, size_t mark) {
    
    for (Command& c : commands) {
        std::cout << "~~ " << Command::print(c.cmd) << std::endl;
        for (obj::Object* oo : r.stack) {
            std::cout << "  -- "; oo->print(); std::cout << std::endl;
        }
        std::cout << "~~" << std::endl;

        switch (c.cmd) {

        case Command::FUN:
        {
            Command::Closure& closure = *(c.closure[0]);
            obj::Object* arg = _exec_closure(r, closure);

            ((Functions::func_t)c.function)(arg, c.object);

            r.stack.push_back(c.object);
            break;
        }
        case Command::VAR:
        {
            r.stack.push_back(r.get_var(c.arg.uint));
            break;
        }
        case Command::VAW:
        {
            r.set_var(c.arg.uint, r.stack.back());
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
            Command::Closure& closure = *(c.closure[0]);
            obj::Object* key = _exec_closure(r, closure);

            obj::Object* cont = r.stack.back();
            obj::Object* val = c.object;

            cont->index(closure.type, key, val);
            
            r.stack.pop_back();
            r.stack.push_back(val);
            break;
        }
        case Command::TUP:
        {
            obj::Object* o = c.object;
            UInt xxx = obj::get<obj::Tuple>(o).v.size();
            obj::Object** lo = &(*(r.stack.end() - xxx));
            o->set(lo);
            //std::cout << "{{"; o->print(); std::cout << "}}" << std::endl;
            //r.stack.erase(r.stack.begin() + mark, r.stack.end());
            r.stack.erase(r.stack.end() - obj::get<obj::Tuple>(o).v.size(), r.stack.end());
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
        case Command::GEN:
        {
            Command::Closure& closure1 = *(c.closure[1]);
            Command::Closure& closure0 = *(c.closure[0]);

            obj::Object* seq = _exec_closure(r, closure1);
            obj::Sequencer& dst = obj::get<obj::Sequencer>(c.object);

            UInt var = c.arg.uint;
            
            dst.v = [seq,var,&r,&closure0](obj::Object* holder, bool& ok) mutable {

                obj::Object* next = seq->next(ok);
                r.set_var(var, next);
                //std::cout << "++ "; next->print(); std::cout << " ++" <<std::endl;

                obj::Object* val = _exec_closure(r, closure0);

                return val;
            };

            r.stack.push_back(c.object);
            break;
        }
        case Command::ARR:
        {
            obj::Object* seq = r.stack.back();
            r.stack.pop_back();
            obj::Object* dst = c.object;

            dst->fill(seq);

            r.stack.push_back(dst);
            break;
        }
        case Command::MAP:
        {
            obj::Object* seq = r.stack.back();
            r.stack.pop_back();
            obj::Object* dst = c.object;

            dst->fill(seq);

            r.stack.push_back(dst);
            break;
        }
        case Command::FLAT:
        {
            obj::Object* seq = r.stack.back();
            r.stack.pop_back();

            obj::SequencerFlatten& fseq = obj::get<obj::SequencerFlatten>(c.object);
            fseq.wrap(seq);

            r.stack.push_back(c.object);
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

void execute(std::vector<Command>& commands, const Type& type, size_t nvars, std::istream& inputs) {

    Runtime rt(nvars);

    obj::Object* toplevel = new obj::SequencerFile(inputs);
    rt.set_var(0, toplevel);

    execute_init(commands);
    execute_run(commands, rt, 0);

    obj::Object* res;
    
    if (rt.stack.size() != 1)
        throw std::runtime_error("Sanity error: did not produce result");

    std::cout << "-=-" << std::endl;
    res = rt.stack.back();
    rt.stack.pop_back();

    obj::get<obj::Sequencer>(res).printx();
    std::cout << std::endl;
}

#endif
