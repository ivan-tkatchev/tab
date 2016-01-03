#ifndef __TAB_FUNCS_HIST_H
#define __TAB_FUNCS_HIST_H

template <typename T>
void hist(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    obj::ArrayAtom<T>& a = obj::get< obj::ArrayAtom<T> >(arg.v[0]);
    UInt _n = obj::get< obj::Atom<UInt> >(arg.v[1]).v;

    size_t n = _n;

    std::vector<size_t> buckets;
    buckets.resize(n);

    T min;
    T max;
    bool first = true;

    for (T i : a.v) {

        if (first) {
            min = i;
            max = i;
            first = false;

        } else if (i < min) {
            min = i;
        } else if (i > max) {
            max = i;
        }
    }

    if (first)
        throw std::runtime_error("Calling 'hist' on an empty array.");

    Real bucketsize = (max - min) / (Real)n;
    
    for (T i : a.v) {
        Real ix = ((i - min) / bucketsize);

        if (ix < 0)
            ix = 0;
        else if (ix >= n)
            ix = n - 1;

        size_t ixx = ix;
        buckets[ixx]++;
    }

    obj::ArrayObject& o = obj::get<obj::ArrayObject>(out);

    o.clear();

    for (size_t i = 0; i < buckets.size(); ++i) {

        obj::Tuple* x = new obj::Tuple;
        x->v.push_back(new obj::Real(i * bucketsize + min));
        x->v.push_back(new obj::UInt(buckets[i]));

        o.v.push_back(x);
    }
}

template <typename T>
void bucket(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& i = obj::get<obj::Tuple>(in);

    T x   = obj::get< obj::Atom<T> >(i.v[0]).v;
    T min = obj::get< obj::Atom<T> >(i.v[1]).v;
    T max = obj::get< obj::Atom<T> >(i.v[2]).v;

    UInt n = obj::get<obj::UInt>(i.v[3]).v;

    T& o = obj::get< obj::Atom<T> >(out).v;

    T bucketsize = (max - min) / n;
    Int xn = (x - min) / bucketsize;

    // HACK
    if (x == max) --xn;

    o = xn * bucketsize + min;
}

void register_hist(Functions& funcs) {

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::UINT) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<UInt>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::INT) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Int>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::REAL) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Real>);

    funcs.add("bucket",
              Type(Type::TUP, { Type(Type::UINT), Type(Type::UINT), Type(Type::UINT), Type(Type::UINT) }),
              Type(Type::UINT),
              bucket<UInt>);

    funcs.add("bucket",
              Type(Type::TUP, { Type(Type::INT), Type(Type::INT), Type(Type::INT), Type(Type::UINT) }),
              Type(Type::INT),
              bucket<Int>);

    funcs.add("bucket",
              Type(Type::TUP, { Type(Type::REAL), Type(Type::REAL), Type(Type::REAL), Type(Type::UINT) }),
              Type(Type::REAL),
              bucket<Real>);
}

#endif
