#ifndef __TAB_FUNCS_FLATTEN_H
#define __TAB_FUNCS_FLATTEN_H

struct SeqFlattenSeq : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* subseq;
    bool subseq_ok;
    
    void wrap(obj::Object* s) {
        seq = s;
        subseq_ok = false;
    }

    obj::Object* next() {

        if (!subseq_ok) {
            subseq = seq->next();

            if (!subseq) return nullptr;

            subseq_ok = true;
        }

        obj::Object* ret = subseq->next();

        if (!ret) {
            subseq_ok = false;
            return next();
        }

        return ret;
    }
};

template <bool SORTED>
struct SeqFlattenVal : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* subseq;
    bool subseq_ok;

    SeqFlattenVal(const Type& t) {
        subseq = obj::make_seq_from<SORTED>(t);

        if (subseq == nullptr)
            throw std::runtime_error("Cannot flatten a sequence of " + Type::print(t));
    }
    
    void wrap(obj::Object* s) {
        seq = s;
        subseq_ok = false;
    }

    obj::Object* next() {

        if (!subseq_ok) {
            obj::Object* i = seq->next();

            if (!i) return nullptr;

            subseq->wrap(i);
            subseq_ok = true;
        }

        obj::Object* ret = subseq->next();

        if (!ret) {
            subseq_ok = false;
            return next();
        }

        return ret;
    }
};

void flatten(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

template <bool SORTED>
Functions::func_t flatten_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ)
        return nullptr;

    Type t2 = unwrap_seq(args);
    ret = wrap_seq(t2);

    if (t2.type == Type::SEQ) {

        obj = new SeqFlattenSeq;

    } else {
        obj = new SeqFlattenVal<SORTED>(t2);
    }

    return flatten;
}


template <bool SORTED>
void register_flatten(Functions& funcs) {

    funcs.add_poly("flatten", flatten_checker<SORTED>);
}

#endif
