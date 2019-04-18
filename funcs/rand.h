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
struct _sample_get {
    TV& operator()(obj::Object* in) {
	return obj::get< obj::Atom<TV> >(in).v;
    }
};

template <>
struct _sample_get<obj::Object*> {
    obj::Object* operator()(obj::Object* in) {
	return in;
    }
};

template <typename TV>
struct _sample_assign {
    void operator()(TV& old, TV& nw) {
	old = nw;
    }
};

template <>
struct _sample_assign<obj::Object*> {
    void operator()(obj::Object*& old, obj::Object* nw) {
	if (old != nullptr) {
	    delete old;

	}
	old = nw->clone();
    }
};

template <typename ARR, typename TV>
void sample(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& arg = obj::get<obj::Tuple>(in);
    UInt _n = obj::get< obj::Atom<UInt> >(arg.v[0]).v;
    obj::Object* seq = arg.v[1];

    std::vector<TV>& vvv = obj::get<ARR>(out).v;

    vvv.clear();

    size_t i = 0;
    std::mt19937_64& gen = get_rand_generator();

    size_t n = _n;

    while (1) {
        obj::Object* next = seq->next();

        if (!next) break;

        auto x = _sample_get<TV>()(next);

        if (i < n) {
	    vvv.emplace_back();
            _sample_assign<TV>()(vvv.back(), x);

        } else if (std::uniform_int_distribution<UInt>(0, i)(gen) < n) {
            _sample_assign<TV>()(vvv[std::uniform_int_distribution<UInt>(0, n-1)(gen)], x);
        }

        ++i;
    }
}

Functions::func_t sample_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& n = args.tuple->at(0);
    const Type& s = args.tuple->at(1);

    if (n != Type(Type::UINT) || s.type != Type::SEQ)
	return nullptr;

    const Type& st = s.tuple->at(0);

    ret = Type(Type::ARR, { st });

    if (st.type == Type::ATOM) {

	switch (st.atom) {
	case Type::INT:
	    obj = new obj::ArrayAtom<Int>;
	    return sample<obj::ArrayAtom<Int>, Int>;
	case Type::UINT:
	    obj = new obj::ArrayAtom<UInt>;
	    return sample<obj::ArrayAtom<UInt>, UInt>;
	case Type::REAL:
	    obj = new obj::ArrayAtom<Real>;
	    return sample<obj::ArrayAtom<Real>, Real>;
	case Type::STRING:
	    obj = new obj::ArrayAtom<std::string>;
	    return sample<obj::ArrayAtom<std::string>, std::string>;
	}

	return nullptr;

    } else {

	obj = new obj::ArrayObject;
	return sample<obj::ArrayObject, obj::Object*>;
    }

    return nullptr;
}


void register_rand(Functions& funcs) {

    funcs.add("rand", Type(), Type(Type::REAL), rand_ru);
    funcs.add("rand", Type(Type::TUP, { Type(Type::REAL), Type(Type::REAL) }), Type(Type::REAL), rand_ru_n);
    funcs.add("rand", Type(Type::TUP, { Type(Type::INT), Type(Type::INT) }), Type(Type::INT), rand_iu_n<obj::Int, Int>);
    funcs.add("rand", Type(Type::TUP, { Type(Type::UINT), Type(Type::UINT) }), Type(Type::UINT), rand_iu_n<obj::UInt, UInt>);

    funcs.add("normal", Type(), Type(Type::REAL), rand_n);
    funcs.add("normal", Type(Type::TUP, { Type(Type::REAL), Type(Type::REAL) }), Type(Type::REAL), rand_n_n);

    funcs.add_poly("sample", sample_checker);
}

#endif
