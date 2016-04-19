#ifndef __TAB_FUNCS_MISC_H
#define __TAB_FUNCS_MISC_H


struct LTuple : public obj::Tuple {

    void print(obj::Printer& p) {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                p.nl();
            }

            x->print(p);
        }
    }
};

void tolower(const obj::Object* in, obj::Object*& out) {

    const std::string& a = obj::get<obj::String>(in).v;
    std::string& b = obj::get<obj::String>(out).v;

    b.clear();
    for (unsigned char c : a) {
        b += std::tolower(c);
    }
}

void toupper(const obj::Object* in, obj::Object*& out) {

    const std::string& a = obj::get<obj::String>(in).v;
    std::string& b = obj::get<obj::String>(out).v;

    b.clear();
    for (unsigned char c : a) {
        b += std::toupper(c);
    }
}

void join_arr_aux(std::string& ret, const std::vector<std::string>& v,
                  const std::string& pref, const std::string& sep, const std::string& suff) {

    ret = pref;

    bool first = true;

    for (const std::string& i : v) {

        if (first) {
            first = false;
        } else {
            ret += sep;
        }

        ret += i;
    }

    ret += suff;
}

void join_arr(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    const std::vector<std::string>& v = obj::get< obj::ArrayAtom<std::string> >(args.v[0]).v;
    const std::string& sep = obj::get<obj::String>(args.v[1]).v;
    std::string& ret = obj::get<obj::String>(out).v;

    static const std::string empty;

    join_arr_aux(ret, v, empty, sep, empty);
}

void join3_arr(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    const std::string& pref = obj::get<obj::String>(args.v[0]).v;
    const std::vector<std::string>& v = obj::get< obj::ArrayAtom<std::string> >(args.v[1]).v;
    const std::string& sep = obj::get<obj::String>(args.v[2]).v;
    const std::string& suff = obj::get<obj::String>(args.v[3]).v;
    std::string& ret = obj::get<obj::String>(out).v;

    join_arr_aux(ret, v, pref, sep, suff);
}

void join_seq_aux(std::string& ret, obj::Object* seq,
                  const std::string& pref, const std::string& sep, const std::string& suff) {

    ret = pref;

    bool first = true;

    while (1) {

        obj::Object* next = seq->next();

        if (!next) break;

        if (first) {
            first = false;
        } else {
            ret += sep;
        }
        
        ret += obj::get<obj::String>(next).v;
    }

    ret += suff;
}

void join_seq(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::Object* seq = args.v[0];
    const std::string& sep = obj::get<obj::String>(args.v[1]).v;
    std::string& ret = obj::get<obj::String>(out).v;

    static const std::string empty;

    join_seq_aux(ret, seq, empty, sep, empty);
}

void join3_seq(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    const std::string& pref = obj::get<obj::String>(args.v[0]).v;
    obj::Object* seq = args.v[1];
    const std::string& sep = obj::get<obj::String>(args.v[2]).v;
    const std::string& suff = obj::get<obj::String>(args.v[3]).v;
    std::string& ret = obj::get<obj::String>(out).v;

    join_seq_aux(ret, seq, pref, sep, suff);
}

void bytes(const obj::Object* in, obj::Object*& out) {

    const std::string& str = obj::get<obj::String>(in).v;
    std::vector<UInt>& v = obj::get< obj::ArrayAtom<UInt> >(out).v;

    v.clear();

    for (unsigned char c : str) {
        v.push_back(c);
    }
}

void bytes_to_string(const obj::Object* in, obj::Object*& out) {

    const std::vector<UInt>& v = obj::get< obj::ArrayAtom<UInt> >(in).v;
    std::string& str = obj::get<obj::String>(out).v;

    str.clear();

    for (UInt c : v) {

        if (c >= 256)
            throw std::runtime_error("Array-to-string only accepts byte (0-255) values.");

        str.push_back((unsigned char)c);
    }
}

void string_hash_p(const obj::Object* in, obj::Object*& out, UInt init, UInt mul) {

    const std::string& str = obj::get<obj::String>(in).v;
    UInt& hash = obj::get<obj::UInt>(out).v;

    hash = init;

    for (unsigned char c : str) {
        hash ^= (UInt)(c);
        hash *= (UInt)mul;
    }
}

constexpr UInt fnv_basis() {
    return (sizeof(UInt) >= 8 ? 0xcbf29ce484222325 : 0x811c9dc5);
}

constexpr UInt fnv_prime() {
    return (sizeof(UInt) >= 8 ? 0x100000001b3 : 0x01000193);
}

void string_hash(const obj::Object* in, obj::Object*& out) {
    string_hash_p(in, out, fnv_basis(), fnv_prime());
}

void cat(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    std::string& ret = obj::get<obj::String>(out).v;

    ret.clear();

    for (obj::Object* i : args.v) {
        ret += obj::get<obj::String>(i).v;
    }
}

Functions::func_t cat_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP)
        return nullptr;

    for (const Type& t : *(args.tuple)) {

        if (!check_string(t))
            return nullptr;
    }

    ret = Type(Type::STRING);
    return cat;
}

void tuple(const obj::Object* in, obj::Object*& out) {
    out = (obj::Object*)in;
}

Functions::func_t tuple_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args == Type(Type::NONE))
        return nullptr;

    ret = args;
    obj = obj::nothing();
    return tuple;
}

void lines(const obj::Object* in, obj::Object*& out) {
    obj::get<LTuple>(out).v = obj::get<obj::Tuple>((obj::Object*)in).v;
}

Functions::func_t lines_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP)
        return nullptr;

    ret = args;
    obj = new LTuple;
    return lines;
}

void register_misc(Functions& funcs) {

    funcs.add("tolower",
              Type(Type::STRING),
              Type(Type::STRING),
              tolower);

    funcs.add("toupper",
              Type(Type::STRING),
              Type(Type::STRING),
              toupper);

    funcs.add("join",
              Type(Type::TUP, { Type(Type::ARR, { Type(Type::STRING) }), Type(Type::STRING) }),
              Type(Type::STRING),
              join_arr);

    funcs.add("join",
              Type(Type::TUP, { Type(Type::SEQ, { Type(Type::STRING) }), Type(Type::STRING) }),
              Type(Type::STRING),
              join_seq);

    funcs.add("join",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::ARR, { Type(Type::STRING) }),
                          Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::STRING),
              join3_arr);

    funcs.add("join",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::SEQ, { Type(Type::STRING) }),
                          Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::STRING),
              join3_seq);

    funcs.add("bytes",
              Type(Type::STRING),
              Type(Type::ARR, { Type(Type::UINT) }),
              bytes);

    funcs.add("string",
              Type(Type::ARR, { Type(Type::UINT) }),
              Type(Type::STRING),
              bytes_to_string);

    funcs.add("hash", Type(Type::STRING), Type(Type::UINT), string_hash);
    
    funcs.add_poly("cat", cat_checker);
    funcs.add_poly("tuple", tuple_checker);
    funcs.add_poly("lines", lines_checker);
}


#endif
