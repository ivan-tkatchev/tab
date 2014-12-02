#ifndef __TAB_FUNCS_MINMAX_H
#define __TAB_FUNCS_MINMAX_H

template <bool MIN, typename T>
struct AtomMinMax : public obj::Atom<T> {

    obj::Object* clone() const {
        AtomMinMax<MIN,T>* ret = new AtomMinMax<MIN,T>;
        ret->v = this->v;
        return ret;
    }
    
    void merge(const obj::Object* o) {
        T tmp = obj::get< obj::Atom<T> >(o).v;

        if ((MIN && tmp < this->v) || (!MIN && tmp > this->v)) {

            this->v = tmp;
        }
    }
};

template <typename T>
void minmax_atom(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& x = obj::get< obj::Atom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);
    y.v = x.v;
}

template <bool MIN, typename T>
void minmax_arr(const obj::Object* in, obj::Object*& out) {
    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    if (x.v.empty()) {

        if (MIN)  {
            throw std::runtime_error("min() of an empty array");
        } else {
            throw std::runtime_error("max() of an empty array");
        }
    }
    
    y.v = x.v[0];

    for (T i : x.v) {

        if ((MIN && i < y.v) || (!MIN && i > y.v)) {

            y.v = i;
        }
    }
}    


template <bool MIN, typename T>
void minmax_seq(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    bool first = true;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        obj::Atom<T>& x = obj::get< obj::Atom<T> >(ret);

        if (first) {
            y.v = x.v;
            first = false;

        } else if ((MIN && x.v < y.v) || (!MIN && x.v > y.v)) {

            y.v = x.v;
        }
    }

    if (first)
        throw std::runtime_error("min() of an empty sequence");
}    


template <bool MIN>
Functions::func_t minmax_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = t;

        switch (t.atom) {
        case Type::INT:
            return minmax_arr<MIN,Int>;
        case Type::UINT:
            return minmax_arr<MIN,UInt>;
        case Type::REAL:
            return minmax_arr<MIN,Real>;
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
            return minmax_seq<MIN,Int>;
        case Type::UINT:
            return minmax_seq<MIN,UInt>;
        case Type::REAL:
            return minmax_seq<MIN,Real>;
        default:
            return nullptr;
        }
        
    } else if (check_numeric(args)) {

        ret = args;

        switch (args.atom) {
        case Type::INT:
            obj = new AtomMinMax<MIN,Int>;
            return minmax_atom<Int>;
        case Type::UINT:
            obj = new AtomMinMax<MIN,UInt>;
            return minmax_atom<UInt>;
        case Type::REAL:
            obj = new AtomMinMax<MIN,Real>;
            return minmax_atom<Real>;
        default:
            return nullptr;
        }
    }

    return nullptr;
}

void register_minmax(Functions& funcs) {

    funcs.add_poly("min", minmax_checker<true>);
    funcs.add_poly("max", minmax_checker<false>);
}

#endif
