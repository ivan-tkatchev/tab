#ifndef __TUP_FUNCS_AVG_H
#define __TUP_FUNCS_AVG_H

struct AtomAvg : public obj::Real {

    size_t n;

    AtomAvg() : n(0) {}
    
    obj::Object* clone() const {
        AtomAvg* ret = new AtomAvg;
        ret->v = v;
        return ret;
    }
    
    void merge(const obj::Object* o) {
        v += obj::get<obj::Real>(o).v;
        n++;
    }

    void merge_end() {
        n++;
        v /= n;
    }
};

struct AtomVar : public obj::Real {

    Real K;
    Real sumv2;
    size_t n;

    AtomVar() : K(0), sumv2(0), n(0) {}
    
    obj::Object* clone() const {
        AtomVar* ret = new AtomVar;
        ret->v = v;
        return ret;
    }

    void merge_start() {
        K = v;
        v = 0;
    }
    
    void merge(const obj::Object* o) {
        Real x = obj::get<obj::Real>(o).v - K;
        v += x;
        sumv2 += (x * x);
        n++;
    }

    void merge_end() {
        n++;
        v = (sumv2 - (v * v)/n)/n;
    }
};

struct AtomStdev : public AtomVar {
    
    obj::Object* clone() const {
        AtomStdev* ret = new AtomStdev;
        ret->v = v;
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

