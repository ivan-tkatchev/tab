#ifndef __TAB_FUNCS_IF_H
#define __TAB_FUNCS_IF_H

void iffun_throw(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::UInt& z = obj::get<obj::UInt>(args.v[0]);

    if (z.v != 0) {
        out = args.v[1];
    } else {
        throw std::runtime_error("if: argument is false");
    }
}


void iffun(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::UInt& z = obj::get<obj::UInt>(args.v[0]);

    if (z.v == 0) {
        out = args.v[2];
    } else {
        out = args.v[1];
    }
}

template <bool SORTED>
void hasfun_map(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::MapObject<SORTED>& map = obj::get< obj::MapObject<SORTED> >(args.v[0]);
    obj::Object* key = args.v[1];
    obj::UInt& r = obj::get<obj::UInt>(out);

    if (map.v.find(key) == map.v.end()) {
        r.v = 0;
    } else {
        r.v = 1;
    }
}

template <typename T>
void hasfun_arratom(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::ArrayAtom<T>& arr = obj::get< obj::ArrayAtom<T> >(args.v[0]);
    obj::Atom<T>& key = obj::get< obj::Atom<T> >(args.v[1]);
    obj::UInt& r = obj::get<obj::UInt>(out);

    for (const auto& i : arr.v) {
        if (i == key.v) {
            r.v = 1;
            return;
        }
    }

    r.v = 0;
}

void hasfun_arrobject(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::ArrayObject& arr = obj::get< obj::ArrayObject >(args.v[0]);
    obj::Object* key = args.v[1];
    obj::UInt& r = obj::get<obj::UInt>(out);

    for (const auto& i : arr.v) {
        if (i->eq(key)) {
            r.v = 1;
            return;
        }
    }

    r.v = 0;
}

void casefun(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    obj::Object* arg = args.v[0];
    obj::Object* def = args.v.back();

    size_t i = 1;
    size_t n = args.v.size() - 1;

    while (i < n) {

        if (arg->eq(args.v[i])) {

            out = args.v[i+1];
            return;
        }

        i += 2;
    }

    out = def;
}

void eqfun(const obj::Object* in, obj::Object*& out) {

    obj::UInt& r = obj::get<obj::UInt>(out);
    obj::Tuple& args = obj::get<obj::Tuple>(in);

    auto i = args.v.begin();
    auto e = args.v.end();
    obj::Object* arg = *i;
    ++i;

    while (i != e) {
        if (arg->eq(*i)) {
            r.v = 1;
            return;
        }

        ++i;
    }

    r.v = 0;
}

template <bool AND>
void andorfun(const obj::Object* in, obj::Object*& out) {

    obj::UInt& r = obj::get<obj::UInt>(out);
    obj::Tuple& args = obj::get<obj::Tuple>(in);

    r.v = (AND ? 1 : 0);

    for (obj::Object* arg : args.v) {
        UInt x = (obj::get<obj::UInt>(arg).v ? 1 : 0);

        if (x != r.v) {
            r.v = (AND ? 0 : 1);
            return;
        }
    }
}

Functions::func_t if_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 2 || args.tuple->size() > 3)
        return nullptr;

    const Type& t1 = args.tuple->at(0);
    const Type& t2 = args.tuple->at(1);

    if (!check_integer(t1))
        return nullptr;

    ret = t2;

    // 'if' will always return an argument.
    obj = obj::nothing();

    if (args.tuple->size() == 2) {
        return iffun_throw;

    } else {
        const Type& t3 = args.tuple->at(2);

        if (t2 != t3)
            return nullptr;
    
        return iffun;
    }
}

template <bool SORTED>
Functions::func_t has_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& t1 = args.tuple->at(0);
    const Type& t2 = args.tuple->at(1);

    if (t1.type == Type::MAP || t1.type == Type::ARR) {

        const Type& key = t1.tuple->at(0);
    
        if (t2 != key)
            return nullptr;

        ret = Type(Type::UINT);

        if (t1.type == Type::ARR) {

            if (key.type == Type::ATOM) {

                switch (key.atom) {
                case Type::UINT:
                    return hasfun_arratom<UInt>;
                case Type::INT:
                    return hasfun_arratom<Int>;
                case Type::REAL:
                    return hasfun_arratom<Real>;
                case Type::STRING:
                    return hasfun_arratom<std::string>;
                }

                return nullptr;

            } else {
                return hasfun_arrobject;
            }

        } else {
            return hasfun_map<SORTED>;
        }

    } else {
        return nullptr;
    }
}

Functions::func_t case_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 4 || (args.tuple->size() % 2) != 0)
        return nullptr;

    const std::vector<Type>& a = *(args.tuple);

    const Type& targ = a.at(0);
    const Type& tchk = a.at(1);
    const Type& tret = a.at(2);
    const Type& tdef = a.back();

    if (tdef != tret || targ != tchk)
        return nullptr;

    size_t i = 3;
    size_t n = a.size() - 1;
    while (i < n) {

        if (targ != a.at(i) || tret != a.at(i+1))
            return nullptr;
        
        i += 2;
    }

    ret = tret;

    return casefun;
}

Functions::func_t eq_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 2)
        return nullptr;

    const std::vector<Type>& a = *(args.tuple);

    const Type& targ = a.at(0);

    for (size_t i = 1; i < a.size(); ++i) {
        if (targ != a.at(i))
            return nullptr;
    }

    ret = Type(Type::UINT);
    return eqfun;
}

template <bool AND>
Functions::func_t and_or_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 2)
        return nullptr;

    for (const auto& t : *(args.tuple)) {

        if (!check_integer(t)) 
            return nullptr;
    }

    ret = Type(Type::UINT);
    
    return andorfun<AND>;
}

template <bool SORTED>
void register_if(Functions& funcs) {

    funcs.add_poly("if", if_checker);
    funcs.add_poly("has", has_checker<SORTED>);
    funcs.add_poly("case", case_checker);
    funcs.add_poly("eq", eq_checker);
    funcs.add_poly("and", and_or_checker<true>);
    funcs.add_poly("or", and_or_checker<false>);
}

#endif
