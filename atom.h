#ifndef __TAB_ATOM_H
#define __TAB_ATOM_H

typedef long Int;
typedef unsigned long UInt;
typedef double Real;

struct String {
    size_t ix;

    bool operator==(String b) const { return ix == b.ix; }
};

namespace std {

template <> struct hash<String> {
    size_t operator()(String x) const { return hash<size_t>()(x.ix); }
};

}

struct Strings {

    std::unordered_map<std::string,size_t> s2i;
    std::unordered_map<size_t,std::string> i2s;
    
    String add(const std::string& s) {
        auto i = s2i.find(s);

        if (i != s2i.end())
            return String{i->second};

        size_t n = s2i.size() + 1;
        s2i.insert(std::make_pair(s, n));
        i2s.insert(std::make_pair(n, s));
        return String{n};
    }

    const std::string& get(String s) {
        auto i = i2s.find(s.ix);

        if (i == i2s.end())
            throw std::runtime_error("Sanity error: uninterned string.");

        return i->second;
    }
};

Strings& strings() {
    static Strings ret;
    return ret;
}


struct Atom {

    enum {
        INT = 0,
        UINT = 1,
        REAL = 2,
        STRING = 3
    } which;

    union {
        Int inte;
        UInt uint;
        Real real;
        String str;
    };

    Atom(Int i = 0) : which(INT), inte(i) {}
    Atom(UInt i) : which(UINT), uint(i)  {}
    Atom(Real i) : which(REAL), real(i)  {}
    Atom(String i) : which(STRING), str(i) {}

    void copy(const Atom& a) {

        switch (a.which) {
        case INT:
            inte = a.inte;
            break;
        case UINT:
            uint = a.uint;
            break;
        case REAL:
            real = a.real;
            break;
        case STRING:
            str = a.str;
            break;
        };

        which = a.which;
    }
    
    Atom(const Atom& a) {        
        copy(a);
    }

    ~Atom() {}

    Atom& operator=(const Atom& a) {
        copy(a);
        return *this;
    }

    bool is_string() const { return (which == STRING); }
    
    static std::string print(const Atom& a) {
        switch (a.which) {
        case INT:
            return "INT(" + std::to_string(a.inte) + ")";
        case UINT:
            return "UINT(" + std::to_string(a.uint) + ")";
        case REAL:
            return "REAL(" + std::to_string(a.real) + ")";
        case STRING:
            return "STRING(" + strings().get(a.str) + ")";
        }
        return ":~(";
    }
};

#endif
