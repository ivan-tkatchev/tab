#ifndef __TAB_FUNCS_COMBO_H
#define __TAB_FUNCS_COMBO_H

template <typename T>
struct SeqCombo : public obj::SeqBase {

    obj::Tuple* holder;
    std::vector<obj::ArrayAtom<T>*> arrays;
    std::vector<size_t> indexes;

    SeqCombo(size_t n) {
        holder = new obj::Tuple;
        holder->v.resize(n);

        for (size_t i = 0; i < n; ++i) {
            holder->v[i] = new obj::Atom<T>;
        }

        arrays.resize(n);
        indexes.resize(n);
    }

    ~SeqCombo() {
        delete holder;
    }

    obj::Object* next() {

        for (size_t i = 0; i < arrays.size(); ++i) {
            obj::ArrayAtom<T>& array = *arrays[i];
            size_t& index = indexes[i];

            if (index >= array.v.size()) {

                if (i == arrays.size() - 1) {
                    return nullptr;
                }

                index = 0;
                ++indexes[i + 1];
            }

            obj::get<obj::Atom<T>>(holder->v[i]).v = array.v[index];
        }

        indexes[0]++;

        return holder;
    }
};

template <typename T>
void combo(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& t = obj::get<obj::Tuple>(in);
    SeqCombo<T>& z = obj::get<SeqCombo<T>>(out);

    for (size_t i = 0; i < t.v.size(); ++i) {
        z.arrays[i] = (obj::ArrayAtom<T>*)t.v[i];
        z.indexes[i] = 0;
    }
}


Functions::func_t combo_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() < 2)
        return nullptr;

    Type tmp(Type::TUP);

    Type first = *(args.tuple->begin());

    if (first.type != Type::ARR || first.tuple->size() != 1 || first.tuple->at(0).type != Type::ATOM) {
        return nullptr;
    }

    for (const Type& t : *(args.tuple)) {

        if (t != first) {
            return nullptr;
        }

        tmp.push(first.tuple->at(0));
    }

    ret = Type(Type::SEQ);
    ret.push(tmp);

    size_t n = args.tuple->size();

    switch (first.tuple->at(0).atom) {
    case Type::UINT:
        obj = new SeqCombo<UInt>(n);
        return combo<UInt>;
    case Type::INT:
        obj = new SeqCombo<Int>(n);
        return combo<Int>;
    case Type::REAL:
        obj = new SeqCombo<Real>(n);
        return combo<Real>;
    case Type::STRING:
        obj = new SeqCombo<std::string>(n);
        return combo<std::string>;
    }

    return nullptr;
}

void register_combo(Functions& funcs) {

    funcs.add_poly("combo", combo_checker);
}


#endif
