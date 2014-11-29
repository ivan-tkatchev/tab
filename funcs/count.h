#ifndef __TAB_FUNCS_COUNT_H
#define __TAB_FUNCS_COUNT_H

void count_seq(const obj::Object* in, obj::Object*& out) {

    UInt& i = obj::get<obj::UInt>(out).v;

    i = 0;
    bool ok = true;

    while (ok) {
        obj::Object next = ((obj::Object*)in)->next(ok);

        if (!next) break;
        
        ++i;
    }
}

template <typename T>
void count_arratom(const obj::Object* in, obj::Object*& out) {

    const auto& arr = obj::get< obj::ArrayAtom<T> >(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = arr.v.size();
}

void count_arr(const obj::Object* in, obj::Object*& out) {

    const auto& arr = obj::get<obj::ArrayObject>(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = arr.v.size();
}

void count_map(const obj::Object* in, obj::Object*& out) {

    const auto& map = obj::get<obj::MapObject>(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = map.v.size();
}

struct CountNull : public obj::SeqBase {

    obj::UInt* i;

    CountNull() {
        i = new obj::UInt(0);
    }

    obj::Object* next(bool& ok) {

        ok = true;
        ++(i->v);
        return i;
    }
};

void count_null(const obj::Object* in, obj::Object*& out) {

    CountNull& v = obj::get<CountNull>(out);
    v.i->v = 0;
}

Functions::func_t count_checker(const Type& args, Type& ret, obj::Object*& out) {

    if (args.type == Type::NONE) {

        ret = Type(Type::SEQ);
        ret.push(Type::UINT);

        out = new CountNull;
        return count_null;
    }
        
    ret = Type(Type::UINT);

    switch (args.type) {
    case Type::SEQ:
        return count_seq;
    case Type::MAP:
        return count_map;
    case Type::ARR:
        switch (args.tuple->at(0).type) {
        case Type::INT:
            return count_arratom<Int>;
        case Type::UINT:
            return count_arratom<UInt>;
        case Type::REAL:
            return count_arratom<Real>;
        case Type::STRING:
            return count_arratom<std::string>;
        default:
            return count_arr;
        }
    default:
        return nullptr;
    }
}


void register_count(Functions& funcs) {

    funcs.add_poly("count", funcs::count_checker);
}

#endif
