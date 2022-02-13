#ifndef __TAB_FUNCS_FILTER_H
#define __TAB_FUNCS_FILTER_H

template <bool WHICH>
struct SeqFilterWhileOne : public obj::SeqBase {

    obj::Object* seq;

    void _wrap(obj::Object* s) {
        seq = s;
    }

    void wrap(obj::Object* s) {
        _wrap(s);
    }

    obj::Object* _next(bool done) {

        while (1) {
            obj::Object* ret = seq->next();

            if (!ret) return ret;

            obj::Tuple& x = obj::get<obj::Tuple>(ret);
            obj::Int& y = obj::get<obj::Int>(x.v[0]);

            if (y.v == 0 && !done) {
                if (WHICH) {
                    continue;
                } else {
                    return nullptr;
                }
            }

            return x.v[1];
        }
    }

    obj::Object* next() {
        return _next(false);
    }
};

struct SeqUntilOne : public SeqFilterWhileOne<true> {

    bool done;

    SeqUntilOne() : SeqFilterWhileOne<true>(), done(false) {}

    void wrap(obj::Object* s) {
        _wrap(s);
        done = false;
    }

    obj::Object* next() {
        obj::Object* ret = _next(done);

        if (ret) {
            done = true;
        }

        return ret;
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
    
    void _wrap(obj::Object* s) {
        seq = s;
    }

    void wrap(obj::Object* s) {
        _wrap(s);
    }

    obj::Object* _next(bool done) {

        while (1) {
            obj::Object* ret = seq->next();

            if (!ret) return ret;

            obj::Tuple& x = obj::get<obj::Tuple>(ret);
            obj::Int& y = obj::get<obj::Int>(x.v[0]);

            if (y.v == 0 && !done) {
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

    obj::Object* next() {
        return _next(false);
    }
};

struct SeqUntilMany : public SeqFilterWhileMany<true> {

    bool done;

    SeqUntilMany(const Type& t) : SeqFilterWhileMany<true>(t), done(false) {}

    void wrap(obj::Object* s) {
        _wrap(s);
        done = false;
    }

    obj::Object* next() {
        obj::Object* ret = _next(done);

        if (ret) {
            done = true;
        }

        return ret;
    }
};

void filter_while_until(const obj::Object* in, obj::Object*& out) {

    out->wrap((obj::Object*)in);
}

template <bool IS_FILTER, bool IS_UNTIL>
Functions::func_t filter_while_until_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::SEQ || !args.tuple || args.tuple->size() != 1)
        return nullptr;

    Type t = args.tuple->at(0);
    
    if (t.type != Type::TUP || !t.tuple || t.tuple->size() <= 1 || !check_integer(t.tuple->at(0))) {
        throw std::runtime_error("'" + std::string(IS_FILTER ? "filter" : (IS_UNTIL ? "until" : "while")) +
                                 "' accepts a sequence of tuples, where the first tuple element is an integer.");
        //return nullptr;
    }

    t.tuple = std::make_shared<std::vector<Type>>(t.tuple->begin()+1, t.tuple->end());

    if (t.tuple->size() == 1) {

        t = t.tuple->at(0);

        if (IS_UNTIL) {
            obj = new SeqUntilOne;
        } else {
            obj = new SeqFilterWhileOne<IS_FILTER>;
        }

    } else {

        if (IS_UNTIL) {
            obj = new SeqUntilMany(t);
        } else {
            obj = new SeqFilterWhileMany<IS_FILTER>(t);
        }
    }

    ret = Type(Type::SEQ);
    ret.push(t);

    return filter_while_until;
}

void register_filter(Functions& funcs) {

    funcs.add_poly("filter", filter_while_until_checker<true, false>);
    funcs.add_poly("while", filter_while_until_checker<false, false>);
    funcs.add_poly("until", filter_while_until_checker<false, true>);
}

#endif

