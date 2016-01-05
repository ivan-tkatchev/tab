#ifndef __TAB_FUNCS_NGRAM_H
#define __TAB_FUNCS_NGRAM_H

template <typename H>
struct SeqNgramBoxed : public obj::SeqBase {

    H* holder;
    obj::Object* seq;
    UInt n;
    UInt m;

    SeqNgramBoxed(UInt _n) : n(_n), m(0) {
        holder = new H;
    }

    ~SeqNgramBoxed() {
        delete holder;
    }

    void wrap(obj::Object* s) {
        seq = s;
        m = 0;
    }

    void wrap(obj::Object* s, UInt _n) {
        wrap(s);
        n = _n;
    }

    obj::Object* next() {

        auto& v = holder->v;

        while (1) {

            obj::Object* o = seq->next();

            if (!o) return nullptr;

            o = o->clone();

            while (m >= n) {
                delete v.front();
                v.erase(v.begin());
                --m;
            }

            v.push_back(o);
            ++m;

            if (m == n)
                return holder;
        }
    }
};


template <typename T>
struct SeqNgramUnboxed : public obj::SeqBase {

    obj::ArrayAtom<T>* holder;
    obj::Object* seq;
    UInt n;
    UInt m;

    SeqNgramUnboxed() : n(0), m(0) {
        holder = new obj::ArrayAtom<T>;
    }

    ~SeqNgramUnboxed() {
        delete holder;
    }

    void wrap(obj::Object* s, UInt _n) {
        seq = s;
        m = 0;
        n = _n;
    }

    obj::Object* next() {

        auto& v = holder->v;

        while (1) {

            obj::Object* o = seq->next();

            if (!o) return nullptr;

            while (m >= n) {
                v.erase(v.begin());
                --m;
            }

            v.push_back(obj::get< obj::Atom<T> >(o).v);
            ++m;

            if (m == n)
                return holder;
        }
    }
};


void ngram_tup(const obj::Object* in, obj::Object*& out) {

    SeqNgramBoxed<obj::Tuple>& v = obj::get< SeqNgramBoxed<obj::Tuple> >(out);

    v.wrap((obj::Object*)in);
}

template <typename T>
void ngram_array(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& inp = obj::get<obj::Tuple>(in);

    T& v = obj::get<T>(out);
    UInt n = obj::get<obj::UInt>(inp.v[1]).v;

    if (n <= 0)
        throw std::runtime_error("Ngrams of 0 length are not allowed.");

    v.wrap(inp.v[0], n);
}

template <size_t N>
Functions::func_t ngram_tup_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;
    
    Type t(Type::TUP);
    for (size_t i = 0; i < N; ++i) {
        t.push(args.tuple->at(0));
    }

    ret = Type(Type::SEQ);
    ret.push(t);

    obj = new SeqNgramBoxed<obj::Tuple>(N);
    return ngram_tup;
}

Functions::func_t ngram_arr_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    if (!check_unsigned(args.tuple->at(1)))
        return nullptr;

    const Type& a = args.tuple->at(0);
    
    if (a.type != Type::SEQ || !a.tuple || a.tuple->size() != 1)
        return nullptr;

    const Type& st = a.tuple->at(0);

    Type t(Type::ARR);
    t.push(st);

    ret = Type(Type::SEQ);
    ret.push(t);

    if (st.type == Type::ATOM) {

        switch (st.atom) {
        case Type::INT:
            obj = new SeqNgramUnboxed<Int>;
            return ngram_array< SeqNgramUnboxed<Int> >;
        case Type::UINT:
            obj = new SeqNgramUnboxed<UInt>;
            return ngram_array< SeqNgramUnboxed<UInt> >;
        case Type::REAL:
            obj = new SeqNgramUnboxed<Real>;
            return ngram_array< SeqNgramUnboxed<Real> >;
        case Type::STRING:
            obj = new SeqNgramUnboxed<std::string>;
            return ngram_array< SeqNgramUnboxed<std::string> >;
        }

        return nullptr;

    } else {

        obj = new SeqNgramBoxed<obj::ArrayObject>(0);
        return ngram_array< SeqNgramBoxed<obj::ArrayObject> >;
    }
}

void register_ngram(Functions& funcs) {

    funcs.add_poly("pairs", ngram_tup_checker<2>);
    funcs.add_poly("triplets", ngram_tup_checker<3>);
    funcs.add_poly("ngrams", ngram_arr_checker);
}

#endif

