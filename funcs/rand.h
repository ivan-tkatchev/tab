#ifndef __TUP_FUNCS_RAND_H
#define __TUP_FUNCS_RAND_H

std::mt19937_64& get_rand_generator(size_t seed = 0) {
    static thread_local std::mt19937_64 ret(seed);
    return ret;
}

void rand_ru(const obj::Object* in, obj::Object*& out) {
    std::uniform_real_distribution<Real> d;
    obj::get<obj::Real>(out).v = d(get_rand_generator());
}

void rand_ru_n(const obj::Object* in, obj::Object*& out) {
    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    std::uniform_real_distribution<Real> d(obj::get<obj::Real>(arg.v[0]).v, obj::get<obj::Real>(arg.v[1]).v);
    obj::get<obj::Real>(out).v = d(get_rand_generator());
}

template <typename T, typename T2>
void rand_iu_n(const obj::Object* in, obj::Object*& out) {
    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    std::uniform_int_distribution<T2> d(obj::get<T>(arg.v[0]).v, obj::get<T>(arg.v[1]).v);
    obj::get<T>(out).v = d(get_rand_generator());
}

void rand_n(const obj::Object* in, obj::Object*& out) {
    std::normal_distribution<Real> d;
    obj::get<obj::Real>(out).v = d(get_rand_generator());
}

void rand_n_n(const obj::Object* in, obj::Object*& out) {
    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    std::normal_distribution<Real> d(obj::get<obj::Real>(arg.v[0]).v, obj::get<obj::Real>(arg.v[1]).v);
    obj::get<obj::Real>(out).v = d(get_rand_generator());
}

template <typename TV>
void sample(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    UInt _n = obj::get< obj::Atom<UInt> >(arg.v[0]).v;
    obj::Object* seq = arg.v[1];

    std::vector<TV>& vvv = obj::get< obj::ArrayAtom<TV> >(out).v;

    vvv.clear();

    size_t i = 0;
    std::mt19937_64& gen = get_rand_generator();

    size_t n = _n;
    
    while (1) {
        obj::Object* next = seq->next();

        if (!next) break;

        TV& x = obj::get< obj::Atom<TV> >(next).v;

        if (i < n) {
            vvv.push_back(x);
        } else if (std::uniform_int_distribution<UInt>(0, i)(gen) < n) {
            vvv[std::uniform_int_distribution<UInt>(0, n-1)(gen)] = x;
        }

        ++i;
    }
}

void register_rand(Functions& funcs) {

    funcs.add("rand", Type(), Type(Type::REAL), rand_ru);
    funcs.add("rand", Type(Type::TUP, { Type(Type::REAL), Type(Type::REAL) }), Type(Type::REAL), rand_ru_n);
    funcs.add("rand", Type(Type::TUP, { Type(Type::INT), Type(Type::INT) }), Type(Type::INT), rand_iu_n<obj::Int, Int>);
    funcs.add("rand", Type(Type::TUP, { Type(Type::UINT), Type(Type::UINT) }), Type(Type::UINT), rand_iu_n<obj::UInt, UInt>);

    funcs.add("normal", Type(), Type(Type::REAL), rand_n);
    funcs.add("normal", Type(Type::TUP, { Type(Type::REAL), Type(Type::REAL) }), Type(Type::REAL), rand_n_n);

    funcs.add("sample",
              Type(Type::TUP, { Type(Type::UINT), Type(Type::SEQ, { Type(Type::UINT) }) }),
              Type(Type::ARR, { Type(Type::UINT) }), sample<UInt>);
    funcs.add("sample",
              Type(Type::TUP, { Type(Type::UINT), Type(Type::SEQ, { Type(Type::INT) }) }),
              Type(Type::ARR, { Type(Type::INT) }), sample<Int>);
    funcs.add("sample",
              Type(Type::TUP, { Type(Type::UINT), Type(Type::SEQ, { Type(Type::REAL) }) }),
              Type(Type::ARR, { Type(Type::REAL) }), sample<Real>);
    funcs.add("sample",
              Type(Type::TUP, { Type(Type::UINT), Type(Type::SEQ, { Type(Type::STRING) }) }),
              Type(Type::ARR, { Type(Type::STRING) }), sample<std::string>);

}

#endif
