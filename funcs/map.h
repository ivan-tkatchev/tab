#ifndef __TAB_FUNCS_MAP_H
#define __TAB_FUNCS_MAP_H

template <bool SORTED>
void map_from_tuple(const obj::Object* in, obj::Object*& out) {
    obj::MapObject<SORTED>& o = obj::get< obj::MapObject<SORTED> >(out);
    obj::Tuple& i = obj::get<obj::Tuple>(in);

    o.v.clear();
    o.v[i.v[0]] = i.v[1];
}

void map_from_seq(const obj::Object* in, obj::Object*& out) {

    out->fill((obj::Object*)in);
}

template <bool SORTED>
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

        return map_from_tuple<SORTED>;
    }

    return nullptr;
}

struct FlipSeq : public obj::SeqBase {

    obj::Tuple* holder;
    obj::Object* seq;

    FlipSeq() {
        holder = new obj::Tuple;
        holder->v.resize(2);
    }

    ~FlipSeq() {
        delete holder;
    }

    obj::Object* next() {

        obj::Object* o = seq->next();

        if (!o) return nullptr;

        obj::Tuple& t = obj::get<obj::Tuple>(o);

        holder->v[0] = t.v[1];
        holder->v[1] = t.v[0];

        return holder;
    }
};

template <bool SORTED>
struct FlipSeqMapObject : public obj::SeqBase {

    obj::Tuple* holder;
    typename obj::MapObject<SORTED>::map_t::const_iterator b;
    typename obj::MapObject<SORTED>::map_t::const_iterator e;

    FlipSeqMapObject() {
        holder = new obj::Tuple;
        holder->v.resize(2);
    }
    
    void wrap(Object* a) {
        obj::MapObject<SORTED>* map = (obj::MapObject<SORTED>*)a;
        b = map->v.begin();
        e = map->v.end();
    }

    obj::Object* next() {

        if (b == e) {
            return nullptr;
        }

        holder->v[1] = b->first;
        holder->v[0] = b->second;
        ++b;

        return holder;
    }
};


void flip_seq(const obj::Object* in, obj::Object*& out) {
    FlipSeq& o = obj::get<FlipSeq>(out);
    o.seq = (obj::Object*)in;
}

template <bool SORTED>
void flip_map(const obj::Object* in, obj::Object*& out) {
    FlipSeqMapObject<SORTED>& o = obj::get< FlipSeqMapObject<SORTED> >(out);
    o.wrap((obj::Object*)in);
}

template <bool SORTED>
Functions::func_t flip_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::SEQ && args.tuple && args.tuple->size() == 1) {

        Type t = args.tuple->at(0);

        if (t.type != Type::TUP || !t.tuple || t.tuple->size() != 2)
            return nullptr;

        std::swap(t.tuple->at(0), t.tuple->at(1));

        ret = Type(Type::SEQ);
        ret.push(t);

        obj = new FlipSeq;
        return flip_seq;
    }

    if (args.type == Type::MAP && args.tuple && args.tuple->size() == 2) {

        Type t(Type::TUP);
        t.push(args.tuple->at(1));
        t.push(args.tuple->at(0));

        ret = Type(Type::SEQ);
        ret.push(t);

        obj = new FlipSeqMapObject<SORTED>;
        return flip_map<SORTED>;
    }

    return nullptr;
}


template <size_t N>
struct NthSeq : public obj::SeqBase {

    obj::Object* seq;

    obj::Object* next() {

        obj::Object* o = seq->next();

        if (!o) return nullptr;

        obj::Tuple& t = obj::get<obj::Tuple>(o);

        return t.v[N];
    }
};

template <bool SORTED, size_t N>
struct NthSeqMapObject : public obj::SeqBase {

    typename obj::MapObject<SORTED>::map_t::const_iterator b;
    typename obj::MapObject<SORTED>::map_t::const_iterator e;
    
    void wrap(Object* a) {
        obj::MapObject<SORTED>* map = (obj::MapObject<SORTED>*)a;
        b = map->v.begin();
        e = map->v.end();
    }

    obj::Object* next() {

        if (b == e) {
            return nullptr;
        }

        obj::Object* o = (N == 0 ? b->first : b->second);
        ++b;

        return o;
    }
};

template <size_t N>
void nth_seq(const obj::Object* in, obj::Object*& out) {
    NthSeq<N>& o = obj::get< NthSeq<N> >(out);
    o.seq = (obj::Object*)in;
}

template <bool SORTED, size_t N>
void nth_map(const obj::Object* in, obj::Object*& out) {
    NthSeqMapObject<SORTED,N>& o = obj::get< NthSeqMapObject<SORTED,N> >(out);
    o.wrap((obj::Object*)in);
}

template <size_t N>
void nth_tup(const obj::Object* in, obj::Object*& out) {
    obj::Tuple& i = obj::get<obj::Tuple>(in);
    out = i.v[N];
}

template <bool SORTED, size_t N>
Functions::func_t nth_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::SEQ && args.tuple && args.tuple->size() == 1) {

        Type t = args.tuple->at(0);

        if (t.type != Type::TUP || !t.tuple || t.tuple->size() != 2)
            return nullptr;

        ret = Type(Type::SEQ);
        ret.push(t.tuple->at(N));

        obj = new NthSeq<N>;
        return nth_seq<N>;
    }

    if (args.type == Type::MAP && args.tuple && args.tuple->size() == 2) {

        ret = Type(Type::SEQ);
        ret.push(args.tuple->at(N));

        obj = new NthSeqMapObject<SORTED,N>;
        return nth_map<SORTED,N>;
    }

    if (args.type == Type::TUP && args.tuple && args.tuple->size() == 2) {

        ret = args.tuple->at(N);
        obj = obj::nothing();
        return nth_tup<N>;
    }
    
    return nullptr;
}


template <bool SORTED>
void register_map(Functions& funcs) {

    funcs.add_poly("map", map_checker<SORTED>);
    funcs.add_poly("flip", flip_checker<SORTED>);
    funcs.add_poly("first", nth_checker<SORTED,0>);
    funcs.add_poly("second", nth_checker<SORTED,1>);
}

#endif

