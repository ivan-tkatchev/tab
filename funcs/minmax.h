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

template <bool MIN>
void minmax_arrobject(const obj::Object* in, obj::Object*& out) {
    obj::ArrayObject& x = obj::get<obj::ArrayObject>(in);

    if (x.v.empty()) {

        if (MIN)  {
            throw std::runtime_error("min() of an empty array");
        } else {
            throw std::runtime_error("max() of an empty array");
        }
    }
    
    out = x.v[0];

    for (obj::Object* i : x.v) {

        if ((MIN && i->less(out)) || (!MIN && out->less(i))) {

            out = i;
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

    if (first) {
        if (MIN) {
            throw std::runtime_error("min() of an empty sequence");
        } else {
            throw std::runtime_error("max() of an empty sequence");
        }
    }
}

template <bool MIN>
void minmax_seqobject(const obj::Object* in, obj::Object*& out) {

    bool first = true;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        if (first) {
            out = ret->clone();
            first = false;

        } else if ((MIN && ret->less(out)) || (!MIN && out->less(ret))) {

            ret = ret->clone();
            delete out;
            out = ret;
        }
    }

    if (first) {
        if (MIN) {
            throw std::runtime_error("min() of an empty sequence");
        } else {
            throw std::runtime_error("max() of an empty sequence");
        }
    }
}    

template <bool MIN>
Functions::func_t minmax_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        const Type& t = args.tuple->at(0);

        ret = t;

        if (t.type == Type::ATOM) {

            switch (t.atom) {
            case Type::INT:
                return minmax_arr<MIN,Int>;
            case Type::UINT:
                return minmax_arr<MIN,UInt>;
            case Type::REAL:
                return minmax_arr<MIN,Real>;
            case Type::STRING:
                return minmax_arr<MIN,std::string>;
            default:
                return nullptr;
            }

        } else {
            obj = obj::nothing();
            return minmax_arrobject<MIN>;
        }
        
    } else if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);

        ret = t;

        if (t.type == Type::ATOM) {
        
            switch (t.atom) {
            case Type::INT:
                return minmax_seq<MIN,Int>;
            case Type::UINT:
                return minmax_seq<MIN,UInt>;
            case Type::REAL:
                return minmax_seq<MIN,Real>;
            case Type::STRING:
                return minmax_seq<MIN,std::string>;
            default:
                return nullptr;
            }

        } else {

            obj = obj::nothing();
            return minmax_seqobject<MIN>;
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
        case Type::STRING:
            obj = new AtomMinMax<MIN,std::string>;
            return minmax_atom<std::string>;
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
