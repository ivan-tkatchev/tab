#ifndef __TAB_FUNCS_ARRAY_H
#define __TAB_FUNCS_ARRAY_H


template <typename T>
struct IArrayAtom : public obj::ArrayAtom<T> {

    obj::Object* clone() const {
        IArrayAtom<T>* ret = new IArrayAtom<T>;
        ret->v = this->v;
        return ret;
    }

    void print(obj::Printer& p) {
        bool first = true;

        for (const T& x : this->v) {
            if (first) {
                first = false;
            } else {
                p.alts();
            }

            p.val(x);
        }
    }
};

struct IArrayObject : public obj::ArrayObject {

    obj::Object* clone() const {
        IArrayObject* ret = new IArrayObject;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void print(obj::Printer& p) {
        bool first = true;

        for (Object* x : v) {
            if (first) {
                first = false;
            } else {
                p.alts();
            }

            x->print(p);
        }
    }
};

template <typename T>
void array_from_atom(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& x = obj::get< obj::Atom<T> >(in);
    obj::ArrayAtom<T>& y = obj::get< obj::ArrayAtom<T> >(out);

    y.v.clear();
    y.v.push_back(x.v);
}

void array_from_tuple(const obj::Object* in, obj::Object*& out) {
    obj::ArrayObject& o = obj::get<obj::ArrayObject>(out);

    o.v.clear();
    o.v.push_back((obj::Object*)in);
}

template <bool SORTED>
void array_from_map(const obj::Object* in, obj::Object*& out) {

    obj::MapObject<SORTED>& a = obj::get< obj::MapObject<SORTED> >(in);
    obj::ArrayObject& o = obj::get<obj::ArrayObject>(out);
    
    typename obj::MapObject<SORTED>::map_t::const_iterator b = a.v.begin();
    typename obj::MapObject<SORTED>::map_t::const_iterator e = a.v.end();

    o.v.clear();
    
    while (b != e) {

        obj::Tuple* tmp = new obj::Tuple;
        tmp->v.resize(2);
        tmp->v[0] = b->first;
        tmp->v[1] = b->second;

        o.v.push_back(tmp);
        
        ++b;
    }
}

void array_from_seq(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);
}

template <typename T>
void array_from_seq(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);
}

template <typename T>
void iarray_from_array_atom(const obj::Object* in, obj::Object*& out) {
    obj::ArrayAtom<T>& o = obj::get< obj::ArrayAtom<T> >(out);
    obj::ArrayAtom<T>& i = obj::get< obj::ArrayAtom<T> >(in);

    o.v.swap(i.v);
}

void iarray_from_array_object(const obj::Object* in, obj::Object*& out) {
    obj::ArrayObject& o = obj::get<obj::ArrayObject>(out);
    obj::ArrayObject& i = obj::get<obj::ArrayObject>(in);

    o.v.swap(i.v);
}

Functions::func_t iarray_from_array_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::ARR) {
        return nullptr;
    }

    const Type& t = args.tuple->at(0);

    ret = Type(Type::ARR);
    ret.push(t);

    if (t.type == Type::ATOM) {

        switch (t.atom) {
        case Type::INT:
            return iarray_from_array_atom<Int>;
        case Type::UINT:
            return iarray_from_array_atom<UInt>;
        case Type::REAL:
            return iarray_from_array_atom<Real>;
        case Type::STRING:
            return iarray_from_array_atom<std::string>;
        }

        return nullptr;

    } else {
        return iarray_from_array_object;
    }
        
    return nullptr;
}

template <bool SORTED>
Functions::func_t array_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::MAP) {

        ret = Type(Type::ARR);

        Type pair(Type::TUP);
        pair.push(args.tuple->at(0));
        pair.push(args.tuple->at(1));

        ret.push(pair);

        return array_from_map<SORTED>;

    } else if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);
        
        ret = Type(Type::ARR);
        ret.push(t);

        if (t.type == Type::ATOM) {

            switch (t.atom) {
            case Type::INT:
                return array_from_seq<Int>;
            case Type::UINT:
                return array_from_seq<UInt>;
            case Type::REAL:
                return array_from_seq<Real>;
            case Type::STRING:
                return array_from_seq<std::string>;
            }

            return nullptr;
            
        } else {
            return array_from_seq;
        }

    } else if (args.type == Type::ATOM) {

        ret = Type(Type::ARR);
        ret.push(args);
        
        switch (args.atom) {
        case Type::INT:
            return array_from_atom<Int>;
        case Type::UINT:
            return array_from_atom<UInt>;
        case Type::REAL:
            return array_from_atom<Real>;
        case Type::STRING:
            return array_from_atom<std::string>;
        }

        return nullptr;

    } else if (args.type == Type::TUP) {

        ret = Type(Type::ARR);
        ret.push(args);
        
        return array_from_tuple;
    }
        
    return nullptr;
}

template <bool SORTED>
Functions::func_t iarray_checker(const Type& args, Type& ret, obj::Object*& obj) {

    Functions::func_t fn;

    if (args.type == Type::ARR) {

        fn = iarray_from_array_checker(args, ret, obj);

    } else {
    
        fn = array_checker<SORTED>(args, ret, obj);
    }

    if (!fn)
            return fn;

    if (ret.type != Type::ARR || ret.tuple->size() != 1)
        return nullptr;

    const Type& e = ret.tuple->at(0);

    if (e.type == Type::ATOM) {

        switch (e.atom) {
        case Type::INT:
            obj = new IArrayAtom<Int>;
            break;

        case Type::UINT:
            obj = new IArrayAtom<UInt>;
            break;

        case Type::REAL:
            obj = new IArrayAtom<Real>;
            break;

        case Type::STRING:
            obj = new IArrayAtom<std::string>;
            break;
        }

    } else {

        obj = new IArrayObject;
    }

    return fn;
}
    
struct SeqTupleAsArrayObject : public obj::SeqBase {

    obj::Tuple* tup;
    typename std::vector<Object*>::const_iterator b;
    typename std::vector<Object*>::const_iterator e;

    void wrap(Object* a) {
        tup = (obj::Tuple*)a;
        b = tup->v.begin();
        e = tup->v.end();
    }

    Object* next() {

        if (b == e) {
            return nullptr;
        }

        Object* ret = *b;
        ++b;

        return ret;
    }
};

void tabulate(const obj::Object* in, obj::Object*& out) {

    SeqTupleAsArrayObject& seq = obj::get<SeqTupleAsArrayObject>(out);

    seq.wrap((obj::Object*)in);
}

void seqfun(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

template <bool SORTED>
Functions::func_t tabulate_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::TUP && args.tuple && args.tuple->size() > 1) {

        const Type& t = args.tuple->at(0);
        
        for (const Type& i : *(args.tuple)) {

            if (t != i)
                return nullptr;
        }

        ret = Type(Type::SEQ);
        ret.push(t);
        obj = new SeqTupleAsArrayObject;
        
        return tabulate;

    } else {

        obj = obj::make_seq_from<SORTED>(args);

        if (!obj)
            return nullptr;

        ret = wrap_seq(args);
        return seqfun;
    }

    return nullptr;
}

template <bool SORTED>
void register_array(Functions& funcs) {

    funcs.add_poly("array", array_checker<SORTED>);
    funcs.add_poly("tabulate", tabulate_checker<SORTED>);
    funcs.add_poly("seq", tabulate_checker<SORTED>);
    funcs.add_poly("iarray", iarray_checker<SORTED>);
}

#endif

