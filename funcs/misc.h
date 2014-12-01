#ifndef __TAB_FUNCS_MISC_H
#define __TAB_FUNCS_MISC_H

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

void join(const obj::Object* in, obj::Object*& out) {

    const obj::Tuple& args = obj::get<obj::Tuple>(in);
    const std::vector<std::string>& v = obj::get< obj::ArrayAtom<std::string> >(args.v[0]).v;
    const std::string& sep = obj::get<obj::String>(args.v[1]).v;
    std::string& ret = obj::get<obj::String>(out).v;

    ret.clear();

    bool first = true;

    for (const std::string& i : v) {

        if (first) {
            first = false;
        } else {
            ret += sep;
        }

        ret += i;
    }
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
    
void register_misc(Functions& funcs) {

    funcs.add("tolower", Type(Type::STRING), Type(Type::STRING), tolower);
    funcs.add("toupper", Type(Type::STRING), Type(Type::STRING), toupper);
    funcs.add("join", Type(Type::TUP, { Type(Type::ARR, { Type(Type::STRING) }), Type(Type::STRING) }), Type(Type::STRING), join);

    funcs.add_poly("cat", cat_checker);
}


#endif
