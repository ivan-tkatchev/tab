#ifndef __TAB_FUNCS_ARRAY_H
#define __TAB_FUNCS_ARRAY_H

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
}

#endif

