#ifndef __TAB_FUNCS_UNFLATTEN_H
#define __TAB_FUNCS_UNFLATTEN_H

struct SeqUnflattened : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* prev;

    SeqUnflattened(const Type& t) {}

    void wrap(obj::Object* s) {
        seq = s;
        prev = nullptr;
    }

    obj::Object* next() {
        obj::Object* ret;

        if (prev) {
            ret = prev;
            prev = nullptr;
            return ret;
        }

        ret = seq->next();

        if (!ret) {
            seq = nullptr;
            return ret;
        }

        obj::Tuple& x = obj::get<obj::Tuple>(ret);
        obj::Int& y = obj::get<obj::Int>(x.v[0]);
        obj::Object* val = x.v[1];

        if (y.v == 1) {
            prev = val;
            return nullptr;
        } 

        return val;
    }
};

struct SeqUnflattenedMany : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* prev;
    obj::Tuple* holder;

    SeqUnflattenedMany(const Type& t) {
        holder = new obj::Tuple;
        holder->v.resize(t.tuple->size());
    }

    ~SeqUnflattenedMany() {
        delete holder;
    }

    void wrap(obj::Object* s) {
        seq = s;
        prev = nullptr;
    }

    obj::Object* next() {
        obj::Object* ret;

        if (prev) {
            ret = prev;
            prev = nullptr;
            return ret;
        }

        ret = seq->next();

        if (!ret) {
            seq = nullptr;
            return ret;
        }

        obj::Tuple& x = obj::get<obj::Tuple>(ret);
        obj::Int& y = obj::get<obj::Int>(x.v[0]);

        for (size_t i = 0; i < holder->v.size(); ++i) {
            holder->v[i] = x.v[i+1];
        }

        if (y.v == 1) {
            prev = holder;
            return nullptr;
        } 

        return holder;
    }
};

template <typename SEQ>
struct SeqUnflatten : public obj::SeqBase {

    SEQ seq;

    SeqUnflatten(const Type& t) : seq(t) {}

    void wrap(obj::Object* s) {
        seq.wrap(s);
    }

    obj::Object* next() {
        if (seq.seq == nullptr) {
            return nullptr;
        }

        return &seq;
    }
};

void unflatten(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

Functions::func_t unflatten_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    Type a = args.tuple->at(0);

    if (a.type != Type::TUP || !a.tuple || a.tuple->size() <= 1 || !check_integer(a.tuple->at(0)))
        return nullptr;

    if (a.tuple->size() == 2) {
        ret = Type(Type::SEQ, { Type(Type::SEQ, { a.tuple->at(1) }) });

        obj = new SeqUnflatten<SeqUnflattened>(a.tuple->at(1));

    } else {
        a.pop_front();
        ret = Type(Type::SEQ, { Type(Type::SEQ, { a }) });

        obj = new SeqUnflatten<SeqUnflattenedMany>(a);
    }

    return unflatten;
}

void register_unflatten(Functions& funcs) {

    funcs.add_poly("unflatten", unflatten_checker);
}

#endif
