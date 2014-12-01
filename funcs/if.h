#ifndef __TAB_FUNCS_IF_H
#define __TAB_FUNCS_IF_H

void iffun(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    obj::UInt& z = obj::get<obj::UInt>(args.v[0]);

    if (z.v == 0) {
        out = args.v[2];
    } else {
        out = args.v[1];
    }
}
    
Functions::func_t if_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 3)
        return nullptr;

    const Type& t1 = args.tuple->at(0);
    const Type& t2 = args.tuple->at(1);
    const Type& t3 = args.tuple->at(2);

    if (!check_integer(t1) || t2 != t3)
        return nullptr;

    ret = t2;

    // 'if' will always return an argument.
    obj = obj::nothing();
    
    return iffun;
}

void register_if(Functions& funcs) {

    funcs.add_poly("if", funcs::if_checker);
}

#endif
