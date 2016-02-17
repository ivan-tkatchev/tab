#ifndef __TAB_FUNCS_COUNT_H
#define __TAB_FUNCS_COUNT_H

void count_seq(const obj::Object* in, obj::Object*& out) {

    UInt& i = obj::get<obj::UInt>(out).v;

    i = 0;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

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

template <bool SORTED>
void count_map(const obj::Object* in, obj::Object*& out) {

    const auto& map = obj::get< obj::MapObject<SORTED> >(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = map.v.size();
}

struct CountNull : public obj::SeqBase {

    obj::UInt* i;
    UInt max;

    CountNull() {
        i = new obj::UInt(0);
        max = 0;
    }

    ~CountNull() {
        delete i;
    }

    obj::Object* next() {

        if (i->v <= max)
            ++(i->v);

        if (i->v > max)
            return nullptr;

        return i;
    }
};

template <typename T>
struct CountLoop : public obj::SeqBase {

    obj::Atom<T>* i;
    T end;
    T increment;

    CountLoop() {
        i = new obj::Atom<T>();
    }

    ~CountLoop() {
        delete i;
    }

    void set(T s, T e, T inc) {
        i->v = s - inc;
        end = e;
        increment = inc;
    }

    obj::Object* next() {

        i->v += increment;

        if ((increment > 0 && i->v > end) ||
            (increment < 0 && i->v < end))
            return nullptr;

        return i;
    }
};

void count_null(const obj::Object* in, obj::Object*& out) {

    CountNull& v = obj::get<CountNull>(out);
    v.i->v = 0;
    v.max = (UInt)(-1);
}

void count_nulln(const obj::Object* in, obj::Object*& out) {

    CountNull& v = obj::get<CountNull>(out);
    UInt max = obj::get< obj::Atom<UInt> >(in).v;
    
    v.i->v = 0;
    v.max = max;
}

template <typename T>
void count_loop(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& a = obj::get<obj::Tuple>(in);
    CountLoop<T>& o = obj::get< CountLoop<T> >(out);

    T s = obj::get< obj::Atom<T> >(a.v[0]).v;
    T e = obj::get< obj::Atom<T> >(a.v[1]).v;
    T i = obj::get< obj::Atom<T> >(a.v[2]).v;

    o.set(s, e, i);
}

void count_string(const obj::Object* in, obj::Object*& out) {
    obj::String& x = obj::get<obj::String>(in);
    obj::UInt& y = obj::get<obj::UInt>(out);
    y.v = x.v.size();
}

template <bool SORTED>
Functions::func_t count_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::NONE) {

        ret = Type(Type::SEQ);
        ret.push(Type::UINT);

        obj = new CountNull;
        return count_null;
    }

    if (args.type == Type::TUP) {

        if (!args.tuple || args.tuple->size() != 3)
            return nullptr;

        const Type& a1 = args.tuple->at(0);
        const Type& a2 = args.tuple->at(1);
        const Type& a3 = args.tuple->at(2);

        if (a1 != a2 || a2 != a3 || !check_numeric(a1))
            return nullptr;

        ret = Type(Type::SEQ);
        ret.push(a1);

        switch (a1.atom) {
        case Type::INT:
            obj = new CountLoop<Int>;
            return count_loop<Int>;
        case Type::UINT:
            obj = new CountLoop<UInt>;
            return count_loop<UInt>;
        case Type::REAL:
            obj = new CountLoop<Real>;
            return count_loop<Real>;
        default:
            return nullptr;
        }

        return nullptr;
    }

    ret = Type(Type::UINT);

    switch (args.type) {
    case Type::SEQ:
        return count_seq;

    case Type::MAP:
        return count_map<SORTED>;

    case Type::ARR:
    {
        const Type& at = args.tuple->at(0);

        if (at.type == Type::ATOM) {
            switch (at.atom) {
            case Type::INT:
                return count_arratom<Int>;
            case Type::UINT:
                return count_arratom<UInt>;
            case Type::REAL:
                return count_arratom<Real>;
            case Type::STRING:
                return count_arratom<std::string>;
            }

        } else {
            return count_arr;
        }
    }

    case Type::ATOM:
        switch (args.atom) {

        case Type::STRING:
            return count_string;

        case Type::UINT:
            ret = Type(Type::SEQ);
            ret.push(Type::UINT);
            obj = new CountNull;
            return count_nulln;

        default:
            return nullptr;
        }
        break;

    default:
        return nullptr;
    }
}


template <bool SORTED>
void register_count(Functions& funcs) {

    funcs.add_poly("count", count_checker<SORTED>);
}

#endif
