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

void take2(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::Object* x = ((obj::Object*)args.v[0])->next();

    if (!x) {
        out = args.v[1];

    } else {

        x = x->clone();

        if (out != args.v[1])
            delete out;

        out = x;
    }
}

void peek(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& ret = obj::get<obj::Tuple>(out);
    obj::Object* x = ((obj::Object*)in)->next();

    if (!x)
        throw std::runtime_error("peek() of an empty sequence.");

    obj::Object* x2 = x->clone();

    if (ret.v[0] != nullptr)
        delete ret.v[0];

    ret.v[0] = x2;
    SeqWithPrev& seq = obj::get<SeqWithPrev>(ret.v[1]);
    seq.wrap((obj::Object*)in);
    seq.prev = x;
}

void glue(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    SeqWithPrev& seq = obj::get<SeqWithPrev>(out);

    seq.wrap(args.v[1]);
    seq.prev = args.v[0];
}

void box(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::Tuple& boxtup = obj::get<obj::Tuple>(out);

    if (boxtup.v.empty()) {
        boxtup.v.push_back(args.v[1]->clone());

    } else if (obj::get<obj::UInt>(args.v[0]).v != 0) {

        obj::Object* x = args.v[1]->clone();
        delete boxtup.v[0];
        boxtup.v[0] = x;
    }
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

    if (args.type == Type::SEQ && args.tuple && args.tuple->size() == 1) {

        ret = args.tuple->at(0);

        return take;
    }

    if (args.type == Type::TUP && args.tuple && args.tuple->size() == 2) {

        const Type& a = args.tuple->at(0);

        if (a.type != Type::SEQ || !a.tuple || a.tuple->size() != 1 || a.tuple->at(0) != args.tuple->at(1))
            return nullptr;

        ret = a.tuple->at(0);

        return take2;
    }

    return nullptr;
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

Functions::func_t box_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    if (!check_unsigned(args.tuple->at(0)))
        return nullptr;

    ret = Type(Type::TUP, { args.tuple->at(1) });

    obj = new obj::Tuple;

    return box;
}

Functions::func_t peek_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    ret = Type(Type::TUP, { args.tuple->at(0), args });

    obj = new obj::Tuple;

    obj::Tuple& t = obj::get<obj::Tuple>(obj);
    t.v.resize(2);
    t.v[0] = nullptr;
    t.v[1] = new SeqWithPrev;

    return peek;
}

void register_explode(Functions& funcs) {

    funcs.add_poly("explode", explode_checker);
    funcs.add_poly("take", take_checker);
    funcs.add_poly("glue", glue_checker);
    funcs.add_poly("box", box_checker);
    funcs.add_poly("peek", peek_checker);
}

#endif
