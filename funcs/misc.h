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

void register_misc(Functions& funcs) {

    funcs.add("tolower", Type(Type::STRING), Type(Type::STRING), tolower);
    funcs.add("toupper", Type(Type::STRING), Type(Type::STRING), toupper);
}


#endif
