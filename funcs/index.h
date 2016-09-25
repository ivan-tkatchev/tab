#ifndef __TAB_FUNCS_INDEX_H
#define __TAB_FUNCS_INDEX_H

size_t __array_ix_conform(size_t vsize, UInt i) {

    return i;
}

size_t __array_ix_conform(size_t vsize, Int z) {

    size_t i = vsize;
    
    if (z < 0)
        z = vsize + z;

    i = z;
    
    return i;
}

size_t __array_ix_conform(size_t vsize, Real z) {

    size_t i = vsize;
    
    if (z >= 0.0 && z <= 1.0)
        i = (vsize - 1)* z;

    return i;
}


template <typename Obj, typename RetT, typename IxT>
struct index_array {

    static void doit(const obj::Object* in, obj::Object*& out) {

        obj::Tuple& args = obj::get<obj::Tuple>(in);
        Obj& a = obj::get<Obj>(args.v[0]);
        IxT& i = obj::get<IxT>(args.v[1]);

        size_t ii = __array_ix_conform(a.v.size(), i.v);

        if (ii >= a.v.size())
            throw std::runtime_error("Array index out of bounds");

        RetT& ret = obj::get<RetT>(out);
        ret.v = a.v[ii];
    }
};

template <typename Obj, typename IxT>
struct index_array<Obj,obj::Object,IxT> {

    static void doit(const obj::Object* in, obj::Object*& out) {

        obj::Tuple& args = obj::get<obj::Tuple>(in);
        Obj& a = obj::get<Obj>(args.v[0]);
        IxT& i = obj::get<IxT>(args.v[1]);

        size_t ii = __array_ix_conform(a.v.size(), i.v);

        if (ii >= a.v.size())
            throw std::runtime_error("Array index out of bounds");

        out = a.v[ii];
    }
};

template <typename Obj, typename IxT1, typename IxT2>
void slice_array(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    Obj& a = obj::get<Obj>(args.v[0]);
    IxT1& i1 = obj::get<IxT1>(args.v[1]);
    IxT2& i2 = obj::get<IxT2>(args.v[2]);

    size_t ii1 = __array_ix_conform(a.v.size(), i1.v);
    size_t ii2 = __array_ix_conform(a.v.size(), i2.v);

    if (ii1 >= a.v.size() || ii2 >= a.v.size())
        throw std::runtime_error("Array index out of bounds");

    if (ii2 < ii1)
        throw std::runtime_error("Array slice indexes are not in order");
    
    Obj& ret = obj::get<Obj>(out);
    ret.v.clear();
    ret.v.assign(a.v.begin() + ii1, a.v.begin() + ii2 + 1);
}


template <typename Obj,typename RetT,typename IxT1>
Functions::func_t index_checker_3(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.tuple->size() == 2) {

        ret = args.tuple->at(0).tuple->at(0);

        return index_array<Obj,RetT,IxT1>::doit;
    }

    const Type& i2 = args.tuple->at(2);

    if (i2.type != Type::ATOM)
        return nullptr;
    
    ret = args.tuple->at(0);

    switch (i2.atom) {
    case Type::UINT:
        return slice_array<Obj,IxT1,obj::UInt>;
    case Type::INT:
        return slice_array<Obj,IxT1,obj::Int>;
    case Type::REAL:
        return slice_array<Obj,IxT1,obj::Real>;
    default:
        return nullptr;
    }
}

template <typename Obj,typename RetT>
Functions::func_t index_checker_2(const Type& args, Type& ret, obj::Object*& obj, bool isobject = false) {

    const Type& i1 = args.tuple->at(1);

    if (i1.type != Type::ATOM)
        return nullptr;

    switch (i1.atom) {
    case Type::UINT:
        return index_checker_3<Obj,RetT,obj::UInt>(args, ret, obj);
    case Type::INT:
        return index_checker_3<Obj,RetT,obj::Int>(args, ret, obj);
    case Type::REAL:
        return index_checker_3<Obj,RetT,obj::Real>(args, ret, obj);
    default:
        return nullptr;
    }
}

template <bool SORTED>
void map_index_one(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject<SORTED>& map = obj::get< obj::MapObject<SORTED> >(args.v[0]);
    obj::Object* key = args.v[1];

    auto i = map.v.find(key);

    if (i == map.v.end())
        throw std::runtime_error("Key is not in map");

    out = i->second;
}

template <bool SORTED>
void map_index_tup(const obj::Object* in, obj::Object*& out) {

    static thread_local obj::Tuple* key = new obj::Tuple;
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject<SORTED>& map = obj::get< obj::MapObject<SORTED> >(args.v[0]);

    key->set(args.v.begin() + 1, args.v.end());

    auto i = map.v.find(key);

    if (i == map.v.end())
        throw std::runtime_error("Key is not in map");

    out = i->second;
}

void tup_index(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::Tuple& tup = obj::get<obj::Tuple>(args.v[0]);
    UInt i = obj::get< obj::Atom<UInt> >(args.v[1]).v;

    out = tup.v[i];
}

template <typename T1,typename T2>
void index_substr(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    const std::string& v = obj::get<obj::String>(args.v[0]).v;
    auto a1 = obj::get<T1>(args.v[1]).v;
    auto a2 = obj::get<T2>(args.v[2]).v;
    std::string& o = obj::get<obj::String>(out).v;

    size_t i1 = __array_ix_conform(v.size(), a1);
    size_t i2 = __array_ix_conform(v.size(), a2);

    if (i1 >= v.size() || i2 >= v.size())
        throw std::runtime_error("Substring index out of bounds");

    if (i2 < i1)
        throw std::runtime_error("Substring indexes are not in order");

    o = v.substr(i1, i2-i1+1);
}

template <bool SORTED>
Functions::func_t index_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() <= 1)
        return nullptr;

    const Type& ci = args.tuple->at(0);

    if (check_string(ci) && args.tuple->size() == 3) {

        const Type& a1 = args.tuple->at(1);
        const Type& a2 = args.tuple->at(2);

        if (!check_integer(a1) || !check_integer(a2))
            return nullptr;

        ret = ci;

        if (check_unsigned(a1) && check_unsigned(a2))
            return index_substr<obj::UInt,obj::UInt>;
        else if (check_unsigned(a1))
            return index_substr<obj::UInt,obj::Int>;
        else if (check_unsigned(a2))
            return index_substr<obj::Int,obj::UInt>;
        else
            return index_substr<obj::Int,obj::Int>;
    }

    if (ci.type == Type::MAP) {

        Type key;
        bool one = true;

        if (args.tuple->size() == 2) {
            key = args.tuple->at(1);

        } else {
            key.type = Type::TUP;

            for (size_t i = 1; i < args.tuple->size(); ++i) {
                key.push(args.tuple->at(i));
            }

            one = false;
        }

        const Type& mkey = ci.tuple->at(0);
        const Type& mval = ci.tuple->at(1);

        if (mkey != key)
            return nullptr;

        obj = obj::nothing();
        ret = mval;

        return (one ? map_index_one<SORTED> : map_index_tup<SORTED>);
    }

    if (ci.type == Type::TUP) {

        if (args.tuple->size() != 2)
            return nullptr;
        
        const Type& arg = args.tuple->at(1);

        if (!arg.literal || !check_unsigned(arg)) 
            throw std::runtime_error("Indexing tuples is only possible with integer literals. (Not expressions!)");

        size_t i = (size_t)arg.literal->uint;

        if (i >= ci.tuple->size())
            throw std::runtime_error("Tuple index out of range");

        obj = obj::nothing();
        ret = ci.tuple->at(i);

        return tup_index;
    }
    
    if (ci.type != Type::ARR || (args.tuple->size() != 2 && args.tuple->size() != 3))
        return nullptr;

    const Type& cci = ci.tuple->at(0);

    if (cci.type == Type::ATOM) {

        switch (cci.atom) {
        case Type::UINT:
            return index_checker_2< obj::ArrayAtom<UInt>,obj::UInt >(args, ret, obj);
        case Type::INT:
            return index_checker_2< obj::ArrayAtom<Int>,obj::Int >(args, ret, obj);
        case Type::REAL:
            return index_checker_2< obj::ArrayAtom<Real>,obj::Real >(args, ret, obj);
        case Type::STRING:
            return index_checker_2< obj::ArrayAtom<std::string>,obj::String >(args, ret, obj);
        }

    } else {

        return index_checker_2< obj::ArrayObject,obj::Object >(args, ret, obj);
    }

    return nullptr;
}

template <bool SORTED>
void map_get(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject<SORTED>& map = obj::get< obj::MapObject<SORTED> >(args.v[0]);
    obj::Object* key = args.v[1];
    obj::Object* val = args.v[2];

    auto i = map.v.find(key);

    if (i == map.v.end()) {
        out = val;
    } else {
        out = i->second;
    }
}

void array_obj_get(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::ArrayObject& arr = obj::get<obj::ArrayObject>(args.v[0]);
    UInt i = obj::get<obj::UInt>(args.v[1]).v;

    if (i >= arr.v.size()) {

        out = args.v[2];

    } else {
        out = arr.v[i];
    }
}

template <typename T>
void array_atom_get(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::ArrayAtom<T>& arr = obj::get< obj::ArrayAtom<T> >(args.v[0]);
    UInt i = obj::get<obj::UInt>(args.v[1]).v;

    if (i >= arr.v.size()) {

        obj::get< obj::Atom<T> >(out).v = obj::get< obj::Atom<T> >(args.v[2]).v;

    } else {
        obj::get< obj::Atom<T> >(out).v = arr.v[i];
    }
}


template <bool SORTED>
Functions::func_t get_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 3)
        return nullptr;

    const Type& ci = args.tuple->at(0);

    Type key = args.tuple->at(1);
    Type val = args.tuple->at(2);

    if (ci.type == Type::MAP) {
    
        const Type& mkey = ci.tuple->at(0);
        const Type& mval = ci.tuple->at(1);

        if (mkey != key || mval != val)
            return nullptr;

        obj = obj::nothing();
        ret = mval;

        return map_get<SORTED>;

    } else if (ci.type == Type::ARR && check_unsigned(key)) {

        if (ci.tuple->at(0) != val)
            return nullptr;

        ret = val;

        if (val.type == Type::ATOM) {
            switch (val.atom) {
            case Type::UINT:
                return array_atom_get<UInt>;
            case Type::INT:
                return array_atom_get<Int>;
            case Type::REAL:
                return array_atom_get<Real>;
            case Type::STRING:
                return array_atom_get<std::string>;
            }

        } else {
            
            obj = obj::nothing();
            return array_obj_get;
        }
    }

    return nullptr;
}

template <bool SORTED>
void register_index(Functions& funcs) {

    funcs.add_poly("index", index_checker<SORTED>);
    funcs.add_poly("get", get_checker<SORTED>);
}

#endif
