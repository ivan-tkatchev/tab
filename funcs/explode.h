#ifndef __TAB_FUNCS_EXPLODE_H
#define __TAB_FUNCS_EXPLODE_H

struct SeqWithPrev : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* prev;

    void wrap(obj::Object* s) {
        seq = s;
        prev = nullptr;
    }

    obj::Object* next() {

        obj::Object* ret = (prev ? prev : seq->next());

        prev = nullptr;

        return ret;
    }
};

struct SeqExplode : public obj::SeqBase {

    SeqWithPrev seq;

    void wrap(obj::Object* s) {
        seq.wrap(s);
    }

    obj::Object* next() {

        obj::Object* ret = seq.seq->next();

        if (!ret) return ret;

        seq.prev = ret;

        return &seq;
    }
};

void explode(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

void take(const obj::Object* in, obj::Object*& out) {

    obj::Object* x = ((obj::Object*)in)->next();

    if (!x)
        throw std::runtime_error("take() of an empty sequence.");

    x = x->clone();
    delete out;
    out = x;
}

void glue(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    SeqWithPrev& seq = obj::get<SeqWithPrev>(out);

    seq.wrap(args.v[1]);
    seq.prev = args.v[0];
}

Functions::func_t explode_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    ret = Type(Type::SEQ);
    ret.push(args);

    obj = new SeqExplode;

    return explode;
}

Functions::func_t take_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    ret = args.tuple->at(0);

    return take;
}

Functions::func_t glue_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& val = args.tuple->at(0);
    const Type& seq = args.tuple->at(1);

    if (seq.type != Type::SEQ || !seq.tuple || seq.tuple->size() != 1)
        return nullptr;

    if (val != seq.tuple->at(0))
        return nullptr;

    ret = seq;
    obj = new SeqWithPrev;
    return glue;
}

void register_explode(Functions& funcs) {

    funcs.add_poly("explode", explode_checker);
    funcs.add_poly("take", take_checker);
    funcs.add_poly("glue", glue_checker);
}

#endif
