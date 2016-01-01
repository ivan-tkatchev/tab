#ifndef __TAB_FUNCS_MAP_H
#define __TAB_FUNCS_MAP_H

void map_from_tuple(const obj::Object* in, obj::Object*& out) {
    obj::MapObject& o = obj::get<obj::MapObject>(out);
    obj::Tuple& i = obj::get<obj::Tuple>(in);

    o.v.clear();
    o.v[i.v[0]] = i.v[1];
}

void map_from_seq(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);
}

Functions::func_t map_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (!args.tuple || args.tuple->size() < 1)
        return nullptr;

    if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);

        if (!t.tuple || t.tuple->size() != 2)
            return nullptr;

        ret = Type(Type::MAP);
        ret.push(t.tuple->at(0));
        ret.push(t.tuple->at(1));

        return map_from_seq;

    } else if (args.type == Type::TUP && args.tuple->size() == 2) {

        ret = Type(Type::MAP);
        ret.push(args.tuple->at(0));
        ret.push(args.tuple->at(1));

        return map_from_tuple;
    }

    return nullptr;
}

void register_map(Functions& funcs) {

    funcs.add_poly("map", map_checker);
}

#endif

