#ifndef __TAB_FUNCS_FILTER_H
#define __TAB_FUNCS_FILTER_H

template <bool WHICH>
struct SeqFilterWhileOne : public obj::SeqBase {

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

            if (y.v == 0) {
                if (WHICH) {
                    continue;
                } else {
                    return nullptr;
                }
            }

            return x.v[1];
        }
    }
};

template <bool WHICH>
struct SeqFilterWhileMany : public obj::SeqBase {

    obj::Object* seq;
    obj::Tuple* holder;

    SeqFilterWhileMany(const Type& t) {
        holder = new obj::Tuple;
        holder->v.resize(t.tuple->size());
    }

    ~SeqFilterWhileMany() {
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

            if (y.v == 0) {
                if (WHICH) {
                    continue;
                } else {
                    return nullptr;
                }
            }

            for (size_t i = 0; i < holder->v.size(); ++i) {
                holder->v[i] = x.v[i+1];
            }

            return holder;
        }
    }
};

void filter_while(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

template <bool WHICH>
Functions::func_t filter_while_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    Type t = args.tuple->at(0);
    
    if (t.type != Type::TUP || !t.tuple || t.tuple->size() <= 1 || !check_integer(t.tuple->at(0))) {
        throw std::runtime_error("'" + std::string(WHICH ? "filter" : "while") +
                                 "' accepts a sequence of tuples, where the first tuple element is an integer.");
        //return nullptr;
    }

    t.tuple->erase(t.tuple->begin());

    if (t.tuple->size() == 1) {

        t = t.tuple->at(0);

        obj = new SeqFilterWhileOne<WHICH>;

    } else {

        obj = new SeqFilterWhileMany<WHICH>(t);
    }

    ret = Type(Type::SEQ);
    ret.push(t);

    return filter_while;
}

void register_filter(Functions& funcs) {

    funcs.add_poly("filter", filter_while_checker<true>);
    funcs.add_poly("while", filter_while_checker<false>);
}

#endif

