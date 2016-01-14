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

Functions::func_t explode_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    ret = Type(Type::SEQ);
    ret.push(args);

    obj = new SeqExplode;

    return explode;
}

void register_explode(Functions& funcs) {

    funcs.add_poly("explode", explode_checker);
}

#endif
