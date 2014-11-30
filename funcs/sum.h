#ifndef __TUP_FUNCS_SUM_H
#define __TUP_FUNCS_SUM_H

template <typename T>
struct AtomSum : public obj::Atom<T> {

    obj::Object* clone() const {
        AtomSum<T>* ret = new AtomSum<T>;
        ret->v = this->v;
        return ret;
    }
    
    void merge(const obj::Object* o) {
        this->v += obj::get< obj::Atom<T> >(o).v;
    }
};

template <typename T>
void sum_atom(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& x = obj::get< obj::Atom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);
    y.v = x.v;
}

template <typename T>
void sum_arr(const obj::Object* in, obj::Object*& out) {
    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    y.v = 0;

    for (T i : x.v) {
        y.v += i;
    }
}    

template <typename T>
void sum_seq(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& y = obj::get< obj::Atom<T> >(out);

    y.v = 0;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        obj::Atom<T>& x = obj::get< obj::Atom<T> >(ret);

        y.v += x.v;
    }
}    

Functions::func_t sum_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = t;

        switch (t.atom) {
        case Type::INT:
            return sum_arr<Int>;
        case Type::UINT:
            return sum_arr<UInt>;
        case Type::REAL:
            return sum_arr<Real>;
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
            return sum_seq<Int>;
        case Type::UINT:
            return sum_seq<UInt>;
        case Type::REAL:
            return sum_seq<Real>;
        default:
            return nullptr;
        }
        
    } else if (check_numeric(args)) {

        ret = args;

        switch (args.atom) {
        case Type::INT:
            obj = new AtomSum<Int>;
            return sum_atom<Int>;
        case Type::UINT:
            obj = new AtomSum<UInt>;
            return sum_atom<UInt>;
        case Type::REAL:
            obj = new AtomSum<Real>;
            return sum_atom<Real>;
        default:
            return nullptr;
        }
    }

    return nullptr;
}

void register_sum(Functions& funcs) {

    funcs.add_poly("sum", funcs::sum_checker);
}


#endif

