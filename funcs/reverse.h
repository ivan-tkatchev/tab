#ifndef __TAB_FUNCS_REVERSE_H
#define __TAB_FUNCS_REVERSE_H

template <typename T>
void reverse_arratom(const obj::Object* in, obj::Object*& out) {

    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    std::reverse(x.v.begin(), x.v.end());
    out = (obj::Object*)in;
}

void reverse_arr(const obj::Object* in, obj::Object*& out) {

    obj::ArrayObject& x = obj::get<obj::ArrayObject>(in);
    std::reverse(x.v.begin(), x.v.end());
    out = (obj::Object*)in;
}

Functions::func_t reverse_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::ARR) {

        ret = args;
        obj = obj::nothing();

        const Type& t = args.tuple->at(0);

        if (t.type == Type::ATOM) {

            switch (t.atom) {
            case Type::INT:
                return reverse_arratom<Int>;
            case Type::UINT:
                return reverse_arratom<UInt>;
            case Type::REAL:
                return reverse_arratom<Real>;
            case Type::STRING:
                return reverse_arratom<std::string>;
            }

            return nullptr;

        } else {

            return reverse_arr;
        }
    }

    return nullptr;
}

void register_reverse(Functions& funcs) {

    funcs.add_poly("reverse", reverse_checker);
}

#endif
