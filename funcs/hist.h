#ifndef __TAB_FUNCS_HIST_H
#define __TAB_FUNCS_HIST_H

template <typename T, typename T2>
void hist(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    obj::ArrayAtom<T>& a = obj::get< obj::ArrayAtom<T> >(arg.v[0]);
    ssize_t _n = obj::get< obj::Atom<T2> >(arg.v[1]).v;

    if (_n <= 0) {
        throw std::runtime_error("Histogram bucket count cannot be zero or less.");
    }

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
        x->v.push_back(new obj::Real((i + 1) * bucketsize + min));
        x->v.push_back(new obj::UInt(buckets[i]));

        o.v.push_back(x);
    }
}

void register_hist(Functions& funcs) {

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::UINT) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<UInt,UInt>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::UINT) }), Type(Type::INT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<UInt,Int>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::INT) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Int,UInt>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::INT) }), Type(Type::INT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Int,Int>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::REAL) }), Type(Type::UINT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Real,UInt>);

    funcs.add("hist",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::REAL) }), Type(Type::INT) }),
              Type(Type::ARR, { Type(Type::TUP, { Type(Type::REAL), Type(Type::UINT) }) }),
              hist<Real,Int>);
}

#endif
