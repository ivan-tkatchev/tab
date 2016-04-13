#ifndef __TUP_FUNCS_AVG_H
#define __TUP_FUNCS_AVG_H

struct AtomAvg : public obj::Real {

    size_t n;

    AtomAvg() : n(1) {}
    
    obj::Object* clone() const {
        AtomAvg* ret = new AtomAvg;
        ret->v = v;
        ret->n = n;
        return ret;
    }
    
    void merge(const obj::Object* o) {
        v += obj::get<obj::Real>(o).v;
        n++;
    }

    void merge_end() {
        v /= n;
        n = 1;
    }
};

struct AtomVar : public obj::Real {

    Real K;
    Real sumv2;
    size_t n;

    AtomVar() : K(0), sumv2(0), n(1) {}
    
    obj::Object* clone() const {
        AtomVar* ret = new AtomVar;
        ret->v = v;
        ret->K = K;
        ret->sumv2 = sumv2;
        ret->n = n;
        return ret;
    }
    
    void merge(const obj::Object* o) {

        if (n == 1) {
            K = v;
            sumv2 = 0;
            v = 0;
        }

        Real x = obj::get<obj::Real>(o).v - K;
        v += x;
        sumv2 += (x * x);
        n++;
    }

    void merge_end() {
        v = (sumv2 - (v * v)/n)/n;
        //n = 1;
    }
};

struct AtomStdev : public AtomVar {
    
    obj::Object* clone() const {
        AtomStdev* ret = new AtomStdev;
        ret->v = v;
        ret->K = K;
        ret->sumv2 = sumv2;
        ret->n = n;
        return ret;
    }

    void merge_end() {
        AtomVar::merge_end();
        v = ::sqrt(v);
    }
};

template <typename T>
void avg_var_stdev(const obj::Object* in, obj::Object*& out) {
    obj::Atom<T>& x = obj::get< obj::Atom<T> >(in);
    obj::Real& y = obj::get<obj::Real>(out);
    y.v = x.v;
}

template <typename T>
void avg_arratom(const obj::Object* in, obj::Object*& out) {

    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    obj::Real& y = obj::get<obj::Real>(out);

    T sum = 0;
    
    for (auto i : x.v) {
        sum += i;
    }

    y.v = ((Real)sum) / x.v.size();
}

template <typename T>
void var_arratom(const obj::Object* in, obj::Object*& out) {

    obj::ArrayAtom<T>& x = obj::get< obj::ArrayAtom<T> >(in);
    obj::Real& y = obj::get<obj::Real>(out);

    if (x.v.empty()) {
        y.v = 0;
        return;
    }

    Real K = x.v[0];
    Real sum = 0;
    Real sum2 = 0;

    for (auto i : x.v) {
        Real x = (Real)i - K;
        sum += x;
        sum2 += (x * x);
    }
        
    size_t n = x.v.size();
    y.v = (sum2 - (sum * sum)/n)/n;
}

template <typename T>
void stdev_arratom(const obj::Object* in, obj::Object*& out) {

    var_arratom<T>(in, out);

    obj::Real& y = obj::get<obj::Real>(out);
    y.v = ::sqrt(y.v);
}
    

template <typename T,typename S>
struct avg_var_stdev_arr;

template <typename S>
struct avg_var_stdev_arr<AtomAvg,S> {
    Functions::func_t operator()() { return avg_arratom<S>; }
};

template <typename S>
struct avg_var_stdev_arr<AtomVar,S> {
    Functions::func_t operator()() { return var_arratom<S>; }
};

template <typename S>
struct avg_var_stdev_arr<AtomStdev,S> {
    Functions::func_t operator()() { return stdev_arratom<S>; }
};


template <typename T>
void avg_seq(const obj::Object* in, obj::Object*& out) {

    obj::Real& y = obj::get<obj::Real>(out);
    y.v = 0;

    T sum = 0;
    size_t n = 0;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        sum += obj::get< obj::Atom<T> >(ret).v;
        ++n;
    }

    y.v = ((Real)sum) / n;
}

template <typename T>
void var_seq(const obj::Object* in, obj::Object*& out) {

    obj::Real& y = obj::get<obj::Real>(out);
    y.v = 0;

    Real K;
    Real sum = 0;
    Real sum2 = 0;
    size_t n = 0;

    while (1) {
        obj::Object* ret = ((obj::Object*)in)->next();

        if (!ret) break;

        T i = obj::get< obj::Atom<T> >(ret).v;

        if (n == 0) {
            K = i;

        } else {

            Real x = (Real)i - K;
            sum += x;
            sum2 += (x * x);
        }

        ++n;
    }

    y.v = (sum2 - (sum * sum)/n)/n;
}

template <typename T>
void stdev_seq(const obj::Object* in, obj::Object*& out) {

    var_seq<T>(in, out);

    obj::Real& y = obj::get<obj::Real>(out);
    y.v = ::sqrt(y.v);
}

template <typename T,typename S>
struct avg_var_stdev_seq;

template <typename S>
struct avg_var_stdev_seq<AtomAvg,S> {
    Functions::func_t operator()() { return avg_seq<S>; }
};

template <typename S>
struct avg_var_stdev_seq<AtomVar,S> {
    Functions::func_t operator()() { return var_seq<S>; }
};

template <typename S>
struct avg_var_stdev_seq<AtomStdev,S> {
    Functions::func_t operator()() { return stdev_seq<S>; }
};


template <typename T>
Functions::func_t avg_var_stdev_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (check_numeric(args)) {

        ret = Type(Type::REAL);
        obj = new T;

        switch (args.atom) {
        case Type::INT:
            return avg_var_stdev<Int>;
        case Type::UINT:
            return avg_var_stdev<UInt>;
        case Type::REAL:
            return avg_var_stdev<Real>;
        default:
            return nullptr;
        }

    } else if (args.type == Type::ARR) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = Type(Type::REAL);
        
        switch (t.atom) {
        case Type::INT:
            return avg_var_stdev_arr<T,Int>()();
        case Type::UINT:
            return avg_var_stdev_arr<T,UInt>()();
        case Type::REAL:
            return avg_var_stdev_arr<T,Real>()();
        default:
            return nullptr;
        }

    } else if (args.type == Type::SEQ) {

        const Type& t = args.tuple->at(0);

        if (!check_numeric(t))
            return nullptr;

        ret = Type(Type::REAL);
        
        switch (t.atom) {
        case Type::INT:
            return avg_var_stdev_seq<T,Int>()();
        case Type::UINT:
            return avg_var_stdev_seq<T,UInt>()();
        case Type::REAL:
            return avg_var_stdev_seq<T,Real>()();
        default:
            return nullptr;
        }
    }

    return nullptr;
}


void register_avg(Functions& funcs) {

    funcs.add_poly("avg", avg_var_stdev_checker<AtomAvg>);
    funcs.add_poly("mean", avg_var_stdev_checker<AtomAvg>);

    funcs.add_poly("var", avg_var_stdev_checker<AtomVar>);
    funcs.add_poly("variance", avg_var_stdev_checker<AtomVar>);

    funcs.add_poly("stdev", avg_var_stdev_checker<AtomStdev>);
    funcs.add_poly("stddev", avg_var_stdev_checker<AtomStdev>);
}


#endif

