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
    
    obj::Object* next() {

        ++i;
        if (i > n)
            return nullptr;

        return seq->next();
    }
};

struct SeqHeadVal : public SeqHeadSeq {

    SeqHeadVal(const Type& t) : SeqHeadSeq() {
        seq = obj::make_seq_from(t);
    }
    
    void wrap(obj::Object* s) {
        seq->wrap(s);
    }
};

struct SeqSkipSeq : public obj::SeqBase {

    obj::Object* seq;
    UInt n;
    UInt i;

    SeqSkipSeq() : n(0), i(0) {}

    void wrap(obj::Object* s) {
        seq = s;
    }
    
    obj::Object* next() {

        while (i < n) {
            obj::Object* ret = seq->next();

            if (!ret) return ret;
        
            ++i;
        }

        return seq->next();
    }
};

struct SeqSkipVal : public SeqSkipSeq {

    SeqSkipVal(const Type& t) : SeqSkipSeq() {
        seq = obj::make_seq_from(t);
    }
    
    void wrap(obj::Object* s) {
        seq->wrap(s);
    }
};

template <typename T>
void head_skip(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& inp = obj::get<obj::Tuple>(in);
    obj::Object* arg = inp.v[0];
    UInt n = obj::get<obj::UInt>(inp.v[1]).v;

    T& seq = obj::get<T>(out);

    seq.n = n;
    seq.i = 0;
    seq.wrap(arg);
}

template <typename TS, typename TV>
Functions::func_t head_skip_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& a2 = args.tuple->at(1);

    if (!check_integer(a2))
        return nullptr;

    const Type& a1 = args.tuple->at(0);

    if (a1.type == Type::SEQ) {

        obj = new TS;
        ret = a1;
        return head_skip<TS>;

    } else if (a1.type == Type::ARR) {

        obj = new TV(a1);
        ret = Type(Type::SEQ);
        ret.push(a1.tuple->at(0));
        return head_skip<TV>;
    }
    
    return nullptr;
}

void register_head(Functions& funcs) {

    funcs.add_poly("head", head_skip_checker<SeqHeadSeq,SeqHeadVal>);
    funcs.add_poly("skip", head_skip_checker<SeqSkipSeq,SeqSkipVal>);
}

#endif
