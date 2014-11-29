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

void map_index_one(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject& map = obj::get<obj::MapObject>(args.v[0]);
    obj::Object* key = args.v[1];

    auto i = map.v.find(key);

    if (i == map.v.end())
        throw std::runtime_error("Key is not in map");

    out = i->second;
}

void map_index_tup(const obj::Object* in, obj::Object*& out) {

    static obj::Tuple* key = new obj::Tuple;
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject& map = obj::get<obj::MapObject>(args.v[0]);

    key->set(args.v.begin() + 1, args.v.end());

    auto i = map.v.find(key);

    if (i == map.v.end())
        throw std::runtime_error("Key is not in map");

    out = i->second;
}

template <typename Obj>
void tup_index(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::Tuple& tup = obj::get<obj::Tuple>(args.v[0]);
    Obj& i = obj::get<Obj>(args.v[1]);

    out = tup.v[i.v];
}

Functions::func_t index_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() <= 1)
        return nullptr;

    const Type& ci = args.tuple->at(0);

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

        ret = mval;

        return (one ? map_index_one : map_index_tup);
    }

    if (ci.type == Type::TUP) {

        if (args.tuple->size() != 2)
            return nullptr;
        
        const Type& arg = args.tuple->at(1);

        if (!arg.literal || !check_integer(arg)) 
            throw std::runtime_error("Indexing tuples is only possible with integer literals.");

        size_t i;
        Functions::func_t fun;
        
        if (arg.atom == Type::UINT) {

            i = (size_t)arg.literal->uint;
            fun = tup_index<obj::UInt>;

        } else if (arg.atom == Type::INT) {

            i = (size_t)arg.literal->inte;
            fun = tup_index<obj::Int>;
        }

        
        if (i >= ci.tuple->size())
            throw std::runtime_error("Tuple index out of range");

        ret = ci.tuple->at(i);

        return fun;
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


void register_index(Functions& funcs) {

    funcs.add_poly("index", funcs::index_checker);
}

#endif
