#ifndef __TAB_FUNCS_HEAD_H
#define __TAB_FUNCS_HEAD_H

struct SeqHeadSeq : public obj::SeqBase {

    obj::Object* seq;
    UInt n;
    UInt i;

    SeqHeadSeq() : n(0), i(0) {}

    void wrap(obj::Object* s) {
        seq = s;
    }
    
    obj::Object* next(bool& ok) {

        obj::Object* ret = seq->next(ok);
        
        ++i;

        if (i >= n)
            ok = false;

        return ret;
    }
};

void head(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& inp = obj::get<obj::Tuple>(in);
    obj::Object* arg = inp.v[0];
    UInt n = obj::get<obj::UInt>(inp.v[1]).v;

    SeqHeadSeq& seq = obj::get<SeqHeadSeq>(out);

    seq.n = n;
    seq.i = 0;
    seq.wrap(arg);
}

Functions::func_t head_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& a2 = args.tuple->at(1);

    if (!check_integer(a2))
        return nullptr;

    const Type& a1 = args.tuple->at(0);

    if (a1.type == Type::SEQ) {

        obj = new SeqHeadSeq;
        ret = a1;
        return head;
    }
    
    return nullptr;
}

void register_head(Functions& funcs) {

    funcs.add_poly("head", funcs::head_checker);
}

#endif
