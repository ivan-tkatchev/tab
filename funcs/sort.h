#ifndef __TAB_FUNCS_SORT_H
#define __TAB_FUNCS_SORT_H

template <typename T>
struct ArrayAtomSort : public obj::ArrayAtom<T> {

    obj::Object* clone() const {
        ArrayAtomSort<T>* ret = new ArrayAtomSort<T>;
        ret->v = this->v;
        return ret;
    }

    void merge_end() {
        std::sort(this->v.begin(), this->v.end());
    }
};

struct ArrayObjectSort : public obj::ArrayObject {

    obj::Object* clone() const {
        ArrayObjectSort* ret = new ArrayObjectSort;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void merge_end() {
        std::sort(v.begin(), v.end(), obj::ObjectLess());
    }
};


template <typename T>
void sort_arratom(const obj::Object* in, obj::Object*& out) {

    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    std::sort(x.v.begin(), x.v.end());

    ArrayAtomSort<T>& o = obj::get< ArrayAtomSort<T> >(out);
    o.v.swap(x.v);
}

void sort_arr(const obj::Object* in, obj::Object*& out) {

    obj::ArrayObject& x = obj::get<obj::ArrayObject>(in);
    std::sort(x.v.begin(), x.v.end(), obj::ObjectLess());

    ArrayObjectSort& o = obj::get<ArrayObjectSort>(out);
    o.v.swap(x.v);
}

void sorted_array_from_tuple(const obj::Object* in, obj::Object*& out) {
    ArrayObjectSort& o = obj::get<ArrayObjectSort>(out);
 
    o.v.clear();
    o.v.push_back((obj::Object*)in);
}

void sorted_array_from_tuple_object(const obj::Object* in, obj::Object*& out) {
    ArrayObjectSort& o = obj::get<ArrayObjectSort>(out);
    const obj::Tuple& i = obj::get<obj::Tuple>(in);

    o.v = i.v;
    o.merge_end();
}

template <typename T>
void sorted_array_from_tuple_atom(const obj::Object* in, obj::Object*& out) {
    ArrayAtomSort<T>& o = obj::get<ArrayAtomSort<T>>(out);
    const obj::Tuple& i = obj::get<obj::Tuple>(in);

    o.v.clear();
    for (obj::Object* ii : i.v) {
        o.v.push_back(obj::get<obj::Atom<T>>(ii).v);
    }
    o.merge_end();
}

template <bool SORTED>
void sort_map(const obj::Object* in, obj::Object*& out) {

    array_from_map<SORTED>(in, out);

    if (!SORTED) {
        obj::ArrayObject& o = obj::get<obj::ArrayObject>(out);
        std::sort(o.v.begin(), o.v.end(), obj::ObjectLess());
    }
}

void sort_seq_arr(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);

    obj::ArrayObject& x = obj::get<obj::ArrayObject>(out);
    std::sort(x.v.begin(), x.v.end(), obj::ObjectLess());
}

template <typename T>
void sort_seq_arr(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);

    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(out);
    std::sort(x.v.begin(), x.v.end());
}

template <bool SORTED>    
Functions::func_t sort_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        ret = args;

        const Type& t = args.tuple->at(0);

        if (t.type == Type::ATOM) {

            switch (t.atom) {
            case Type::INT:
                obj = new ArrayAtomSort<Int>;
                return sort_arratom<Int>;
            case Type::UINT:
                obj = new ArrayAtomSort<UInt>;
                return sort_arratom<UInt>;
            case Type::REAL:
                obj = new ArrayAtomSort<Real>;
                return sort_arratom<Real>;
            case Type::STRING:
                obj = new ArrayAtomSort<std::string>;
                return sort_arratom<std::string>;
            }

            return nullptr;

        } else {

            obj = new ArrayObjectSort;
            return sort_arr;
        }

    } else if (args.type == Type::MAP) {

        ret = Type(Type::ARR);

        Type pair(Type::TUP);
        pair.push(args.tuple->at(0));
        pair.push(args.tuple->at(1));

        ret.push(pair);

        obj = new ArrayObjectSort;
        return sort_map<SORTED>;

    } else if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);
        
        ret = Type(Type::ARR);
        ret.push(t);

        if (t.type == Type::ATOM) {

            switch (t.atom) {
            case Type::INT:
                obj = new ArrayAtomSort<Int>;
                return sort_seq_arr<Int>;
            case Type::UINT:
                obj = new ArrayAtomSort<UInt>;
                return sort_seq_arr<UInt>;
            case Type::REAL:
                obj = new ArrayAtomSort<Real>;
                return sort_seq_arr<Real>;
            case Type::STRING:
                obj = new ArrayAtomSort<std::string>;
                return sort_seq_arr<std::string>;
            }

            return nullptr;
            
        } else {

            obj = new ArrayObjectSort;
            return sort_seq_arr;
        }

    } else if (args.type == Type::ATOM) {

        ret = Type(Type::ARR);
        ret.push(args);

        switch (args.atom) {
        case Type::INT:
            obj = new ArrayAtomSort<Int>;
            return array_from_atom<Int>;
        case Type::UINT:
            obj = new ArrayAtomSort<UInt>;
            return array_from_atom<UInt>;
        case Type::REAL:
            obj = new ArrayAtomSort<Real>;
            return array_from_atom<Real>;
        case Type::STRING:
            obj = new ArrayAtomSort<std::string>;
            return array_from_atom<std::string>;
        }

        return nullptr;

    } else if (args.type == Type::TUP) {

        Type first = *(args.tuple->begin());

        for (const Type& t : *(args.tuple)) {

            if (t != first) {
                return nullptr;
            }
        }

        ret = Type(Type::ARR);
        ret.push(first);

        if (first.type == Type::ATOM) {
            switch (first.atom) {
            case Type::INT:
                obj = new ArrayAtomSort<Int>;
                return sorted_array_from_tuple_atom<Int>;
            case Type::UINT:
                obj = new ArrayAtomSort<UInt>;
                return sorted_array_from_tuple_atom<UInt>;
            case Type::REAL:
                obj = new ArrayAtomSort<Real>;
                return sorted_array_from_tuple_atom<Real>;
            case Type::STRING:
                obj = new ArrayAtomSort<std::string>;
                return sorted_array_from_tuple_atom<std::string>;
            }

        } else {
            obj = new ArrayObjectSort;
            return sorted_array_from_tuple_object;
        }
    }
        
    return nullptr;
}

template <bool SORTED>    
Functions::func_t sorted_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::TUP) {

        ret = Type(Type::ARR);
        ret.push(args);

        obj = new ArrayObjectSort;
        return sorted_array_from_tuple;
    }

    return sort_checker<SORTED>(args, ret, obj);
}
    
template <bool SORTED>    
void register_sort(Functions& funcs) {

    funcs.add_poly("sort", sort_checker<SORTED>);
    funcs.add_poly("sorted", sorted_checker<SORTED>);
}

#endif
