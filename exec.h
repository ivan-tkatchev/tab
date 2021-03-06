#ifndef __TAB_EXEC_H
#define __TAB_EXEC_H

namespace tab {

struct Runtime {
    std::vector<obj::Object*> vars;
    std::vector<obj::Object*> stack;

    void init(size_t nvars) {
        vars.resize(nvars);
    }
    
    void set_var(UInt ix, obj::Object* o) {
        vars[ix] = o;
    }

    obj::Object* get_var(UInt ix) {
        return vars[ix];
    }
};

template <bool SORTED>
void execute_init(std::vector<Command>& commands) {

    for (auto& c : commands) {

        for (auto& clo : c.closure) {
            execute_init<SORTED>(clo.code);
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

        case Command::SEQ:
        case Command::ROT:
        case Command::VAW:
        case Command::LAMD:
            break;

        case Command::FUN:
        case Command::FUN0:
            if (c.object == nullptr)
                c.object = obj::make<SORTED>(c.type);
            break;

        case Command::GEN:
        case Command::GEN_TRY:
            c.object = new obj::SeqGenerator;
            break;

        case Command::REC:
            c.object = new obj::Tuple;
            obj::get<obj::Tuple>(c.object).v.resize(2);
            break;

        default:
            c.object = obj::make<SORTED>(c.type);
            break;
        }
    }
}


void execute_run(std::vector<Command>& commands, Runtime& r) {
    
    for (Command& c : commands) {
        switch (c.cmd) {

        case Command::FUN:
        {
            obj::Object* arg = r.stack.back();
            r.stack.pop_back();

            ((Functions::func_t)c.function)(arg, c.object);

            r.stack.push_back(c.object);
            break;
        }
        case Command::FUN0:
        {
            ((Functions::func_t)c.function)(nullptr, c.object);
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
        case Command::TUP:
        {
            obj::Tuple& tup = obj::get<obj::Tuple>(c.object);
            UInt nelem = c.arg.uint;
            auto b = r.stack.end() - nelem;
            auto e = r.stack.end();
            tup.set(b, e);
            r.stack.erase(b, e);
            r.stack.push_back(c.object);
            break;
        }
        case Command::SEQ:
        {
            obj::Object* src = r.stack.back();
            r.stack.pop_back();
            c.object->wrap(src);
            r.stack.push_back(c.object);
            break;
        }
        case Command::GEN:
        {
            obj::Object* seq = r.stack.back();
            r.stack.pop_back();

            Command::Closure& clo = c.closure[0];
            UInt var = c.arg.uint;

            obj::SeqGenerator& gen = obj::get<obj::SeqGenerator>(c.object);

            gen.v = [seq,&clo,var,&r]() mutable {

                obj::Object* next = seq->next();

                if (!next) return next;

                r.set_var(var, next);

                execute_run(clo.code, r);

                obj::Object* val = r.stack.back();
                r.stack.pop_back();

                return val;
            };

            r.stack.push_back(c.object);
            break;
        }
        case Command::GEN_TRY:
        {
            obj::Object* seq = r.stack.back();
            r.stack.pop_back();

            Command::Closure& clo = c.closure[0];
            UInt var = c.arg.uint;

            obj::SeqGenerator& gen = obj::get<obj::SeqGenerator>(c.object);

            gen.v = [seq,&clo,var,&r]() mutable {

                while (1) {

                    obj::Object* next = seq->next();

                    if (!next) return next;

                    r.set_var(var, next);

                    size_t oldsize = r.stack.size();

                    try {
                        execute_run(clo.code, r);

                        obj::Object* val = r.stack.back();
                        r.stack.pop_back();
                        return val;

                    } catch (...) {
                        r.stack.resize(oldsize);
                    }
                }
            };

            r.stack.push_back(c.object);
            break;
        }
        case Command::REC:
        {
            obj::Object* _in = r.stack.back();
            r.stack.pop_back();

            obj::Tuple& in = obj::get<obj::Tuple>(_in);
            obj::Tuple& work = obj::get<obj::Tuple>(c.object);
            UInt var = c.arg.uint;
            r.set_var(var, &work);

            Command::Closure& clo = c.closure[0];

            work.v[0] = in.v[0]->clone();

            while (1) {

                obj::Object* next = in.v[1]->next();

                if (!next) break;

                work.v[1] = next;

                execute_run(clo.code, r);

                obj::Object* cloned = r.stack.back()->clone();
                r.stack.pop_back();
                delete work.v[0];
                work.v[0] = cloned;
            }

            r.stack.push_back(work.v[0]);
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

        case Command::EQ:
        {
            obj::Object* a = r.stack.back();
            r.stack.pop_back();
            obj::Object* b = r.stack.back();
            r.stack.pop_back();
            obj::UInt& x = obj::get<obj::UInt>(c.object);
            x.v = (b->eq(a) ? 1 : 0);
            r.stack.push_back(c.object);
            break;
        }

        case Command::LT:
        {
            obj::Object* a = r.stack.back();
            r.stack.pop_back();
            obj::Object* b = r.stack.back();
            r.stack.pop_back();
            obj::UInt& x = obj::get<obj::UInt>(c.object);
            x.v = (b->less(a) ? 1 : 0);
            r.stack.push_back(c.object);
            break;
        }

        case Command::NEG:
        {
            obj::UInt& x = obj::get<obj::UInt>(r.stack.back());
            x.v = (x.v == 0 ? 1 : 0);
            break;
        }

        case Command::ROT:
        {
            obj::Object* a = r.stack.back();
            r.stack.pop_back();
            obj::Object* b = r.stack.back();
            r.stack.pop_back();
            r.stack.push_back(a);
            r.stack.push_back(b);
            break;
        }
        
        // And here comes the numeric operator boilerplate.

#define MATHOP(TYPE,EXPR)                               \
        TYPE& a = obj::get<TYPE>(r.stack.back());       \
        r.stack.pop_back();                             \
        TYPE& b = obj::get<TYPE>(r.stack.back());       \
        r.stack.pop_back();                             \
        TYPE& x = obj::get<TYPE>(c.object);             \
        x.v = EXPR;                                     \
        r.stack.push_back(c.object);

        case Command::EXP:
        {
            MATHOP(obj::Real, ::pow(b.v, a.v));
            break;
        }
        case Command::MUL_R:
        {
            MATHOP(obj::Real, b.v * a.v);
            break;
        }
        case Command::MUL_I:
        {
            MATHOP(obj::Int, b.v * a.v);
            break;
        }
        case Command::DIV_R:
        {
            MATHOP(obj::Real, b.v / a.v);
            break;
        }
        case Command::DIV_I:
        {
            MATHOP(obj::Int, b.v / a.v);
            break;
        }
        case Command::MOD:
        {
            MATHOP(obj::Int, b.v % a.v);
            break;
        }
        case Command::ADD_R:
        {
            MATHOP(obj::Real, b.v + a.v);
            break;
        }
        case Command::ADD_I:
        {
            MATHOP(obj::Int, b.v + a.v);
            break;
        }
        case Command::SUB_R:
        {
            MATHOP(obj::Real, b.v - a.v);
            break;
        }
        case Command::SUB_I:
        {
            MATHOP(obj::Int, b.v - a.v);
            break;
        }
        case Command::AND:
        {
            MATHOP(obj::Int, b.v & a.v);
            break;
        }
        case Command::OR:
        {
            MATHOP(obj::Int, b.v | a.v);
            break;
        }
        case Command::XOR:
        {
            MATHOP(obj::Int, b.v ^ a.v);
            break;
        }

#undef MATHOP

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
            obj::Int& a = obj::get<obj::Int>(r.stack.back());
            r.stack.pop_back();
            obj::Int& b = obj::get<obj::Int>(c.object);
            b.v = ~a.v;
            r.stack.push_back(c.object);
            break;
        }

        case Command::LAMD:
            // This opcode is a no-op.
            break;
        }
    }
}

template <bool SORTED>
obj::Object* execute(std::vector<Command>& commands, Runtime& rt, obj::Object* input) {

    rt.set_var(0, input);
    rt.stack.clear();

    execute_run(commands, rt);

    if (rt.stack.size() != 1)
        throw std::runtime_error("Sanity error: did not produce result");

    return rt.stack.back();
}

} // namespace tab

#endif
