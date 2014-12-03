#ifndef __TAB_FUNCS_FILTER_H
#define __TAB_FUNCS_FILTER_H

struct SeqFilterOne : public obj::SeqBase {

    obj::Object* seq;
    
    void wrap(obj::Object* s) {
        seq = s;
    }

    obj::Object* next() {

        while (1) {
            obj::Object* ret = seq->next();

            if (!ret) return ret;

            obj::Tuple& x = obj::get<obj::Tuple>(ret);
            obj::Int& y = obj::get<obj::Int>(x.v[0]);

            if (y.v == 0)
                continue;

            return x.v[1];
        }
    }
};

struct SeqFilterMany : public obj::SeqBase {

    obj::Object* seq;
    obj::Tuple* holder;

    SeqFilterMany(const Type& t) {
        holder = new obj::Tuple;
        holder->v.resize(t.tuple->size());
    }

    ~SeqFilterMany() {
        delete holder;
    }
    
    void wrap(obj::Object* s) {
        seq = s;
    }

    obj::Object* next() {

        while (1) {
            obj::Object* ret = seq->next();

            if (!ret) return ret;

            obj::Tuple& x = obj::get<obj::Tuple>(ret);
            obj::Int& y = obj::get<obj::Int>(x.v[0]);

            if (y.v == 0)
                continue;

            for (size_t i = 0; i < holder->v.size(); ++i) {
                holder->v[i] = x.v[i+1];
            }

            return holder;
        }
    }
};

void filter(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

Functions::func_t filter_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ)
        return nullptr;

    Type t = unwrap_seq(args);
    
    if (t.type != Type::TUP || !t.tuple || t.tuple->size() <= 1 || !check_integer(t.tuple->at(0))) {
        throw std::runtime_error("'filter' accepts a sequence of tuples, where the first tuple element is an integer.");
        //return nullptr;
    }

    t.tuple->erase(t.tuple->begin());

    if (t.tuple->size() == 1) {

        t = t.tuple->at(0);

        obj = new SeqFilterOne;

    } else {
    
        obj = new SeqFilterMany(t);
    }

    ret = Type(Type::SEQ);
    ret.push(t);

    return filter;
}


void register_filter(Functions& funcs) {

    funcs.add_poly("filter", filter_checker);
}

#endif

