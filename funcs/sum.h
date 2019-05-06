#ifndef __TUP_FUNCS_SUM_H
#define __TUP_FUNCS_SUM_H

template <bool MUL, typename T>
struct AtomSumMul : public obj::Atom<T> {

    obj::Object* clone() const {
        AtomSumMul<MUL, T>* ret = new AtomSumMul<MUL, T>;
        ret->v = this->v;
        return ret;
    }
    
    void merge(const obj::Object* o) {
	if (MUL) {
	    this->v *= obj::get< obj::Atom<T> >(o).v;
	} else {
	    this->v += obj::get< obj::Atom<T> >(o).v;
	}
    }
};

template <typename T>
void sum_mul_atom(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& x = obj::get< obj::Atom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);
    y.v = x.v;
}

template <bool MUL, typename T>
void sum_mul_arr(const obj::Object* in, obj::Object*& out) {
    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    y.v = (MUL ? 1 : 0);

    for (T i : x.v) {
	if (MUL) {
	    y.v *= i;
	} else {
	    y.v += i;
	}
    }
}    

template <bool MUL, typename T>
void sum_mul_seq(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    y.v = (MUL ? 1 : 0);

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        obj::Atom<T>& x = obj::get< obj::Atom<T> >(ret);

	if (MUL) {
	    y.v *= x.v;
	} else {
	    y.v += x.v;
	}
    }
}    

template <bool MUL>
Functions::func_t sum_mul_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = t;

        switch (t.atom) {
        case Type::INT:
            return sum_mul_arr<MUL, Int>;
        case Type::UINT:
            return sum_mul_arr<MUL, UInt>;
        case Type::REAL:
            return sum_mul_arr<MUL, Real>;
        default:
            return nullptr;
        }
        
    } else if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = t;

        switch (t.atom) {
        case Type::INT:
            return sum_mul_seq<MUL, Int>;
        case Type::UINT:
            return sum_mul_seq<MUL, UInt>;
        case Type::REAL:
            return sum_mul_seq<MUL, Real>;
        default:
            return nullptr;
        }
        
    } else if (check_numeric(args)) {

        ret = args;

        switch (args.atom) {
        case Type::INT:
            obj = new AtomSumMul<MUL, Int>;
            return sum_mul_atom<Int>;
        case Type::UINT:
            obj = new AtomSumMul<MUL, UInt>;
            return sum_mul_atom<UInt>;
        case Type::REAL:
            obj = new AtomSumMul<MUL, Real>;
            return sum_mul_atom<Real>;
        default:
            return nullptr;
        }
    }

    return nullptr;
}


template <bool MUL, typename OBJ>
void add_mul_func(const obj::Object* in, obj::Object*& out) {

    OBJ& r = obj::get<OBJ>(out);
    obj::Tuple& args = obj::get<obj::Tuple>(in);

    r.v = (MUL ? 1 : 0);

    for (obj::Object* arg : args.v) {
	if (MUL) {
	    r.v *= obj::get<OBJ>(arg).v;
	} else {
	    r.v += obj::get<OBJ>(arg).v;
	}
    }
}

template <bool MUL>
Functions::func_t add_mul_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 2)
        return nullptr;

    for (const auto& t : *(args.tuple)) {

        if (check_numeric(t)) {

	    ret = t;

	    switch (args.atom) {
	    case Type::INT:
		return add_mul_func<MUL, obj::Int>;
	    case Type::UINT:
		return add_mul_func<MUL, obj::UInt>;
	    case Type::REAL:
		return add_mul_func<MUL, obj::Real>;
	    default:
		return nullptr;
	    }

	} else {
            return nullptr;
	}
    }

    return nullptr;
}


void register_sum(Functions& funcs) {

    funcs.add_poly("sum", sum_mul_checker<false>);
    funcs.add_poly("product", sum_mul_checker<true>);
    funcs.add_poly("add", add_mul_checker<false>);
    funcs.add_poly("mul", add_mul_checker<true>);
}


#endif

