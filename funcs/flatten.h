#ifndef __TAB_FUNCS_FLATTEN_H
#define __TAB_FUNCS_FLATTEN_H

struct SeqFlattenSeq : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* subseq;
    bool seq_ok;
    bool subseq_ok;
    
    void wrap(obj::Object* s) {
        seq = s;
        subseq_ok = false;
    }

    obj::Object* next(bool& ok) {

        if (!subseq_ok) {
            subseq = seq->next(seq_ok);

            if (!subseq) return subseq;
        }

        obj::Object* ret = subseq->next(subseq_ok);

        if (!ret) {
            if (!seq_ok) {
                return nullptr;
            } else {
                subseq_ok = false;
                return next(ok);
            }
        }

        if (!seq_ok && !subseq_ok) {
            ok = false;
        } else {
            ok = true;
        }

        return ret;
    }
};

struct SeqFlattenVal : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* subseq;
    bool seq_ok;
    bool subseq_ok;

    SeqFlattenVal(const Type& t) {
        subseq = obj::make_seq_from(t);

        if (subseq == nullptr)
            throw std::runtime_error("Cannot flatten a sequence of " + Type::print(t));
    }
    
    void wrap(obj::Object* s) {
        seq = s;
        subseq_ok = false;
    }

    obj::Object* next(bool& ok) {

        if (!subseq_ok) {
            obj::Object* i = seq->next(seq_ok);

            if (!i) return i;
            
            subseq->wrap(i);
        }

        obj::Object* ret = subseq->next(subseq_ok);

        if (!ret) {
            if (!seq_ok) {
                return nullptr;
            } else {
                subseq_ok = false;
                return next(ok);
            }
        }

        if (!seq_ok && !subseq_ok) {
            ok = false;
        } else {
            ok = true;
        }

        return ret;
    }
};

void flatten(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

Functions::func_t flatten_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ)
        return nullptr;

    Type t2 = unwrap_seq(args);
    ret = wrap_seq(t2);

    if (t2.type == Type::SEQ) {

        obj = new SeqFlattenSeq;

    } else {
        obj = new SeqFlattenVal(t2);
    }

    return flatten;
}


void register_flatten(Functions& funcs) {

    funcs.add_poly("flatten", funcs::flatten_checker);
}

#endif
