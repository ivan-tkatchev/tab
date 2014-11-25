#ifndef __TAB_TYPE_H
#define __TAB_TYPE_H
   
struct Type {

    enum types_t {
        ATOM,
        TUP,
        ARR,
        MAP,
        SEQ,
        NONE
    };
    
    enum atom_types_t {
        INT,
        UINT,
        REAL,
        STRING
    };

    types_t type;
    atom_types_t atom;
    std::shared_ptr< std::vector<Type> > tuple;
    
    Type(types_t t = NONE) : type(t), atom(INT) {}

    Type(atom_types_t a) : type(ATOM), atom(a) {}
        
    Type(const Atom& a) : type(ATOM) {
        switch (a.which) {
        case INT: atom = INT; break;
        case UINT: atom = UINT; break;
        case REAL: atom = REAL; break;
        case STRING: atom = STRING; break;
        }
    }

    Type(types_t t, const std::initializer_list<Type>& tup) :
        type(t),
        tuple(std::make_shared< std::vector<Type> >(tup)) {}

    
    bool operator!=(const Type& t) const {
        if (t.type != type) return true;

        if (type == ATOM) {

            if (atom != t.atom)
                return true;

            return false;
        }
            
        if (tuple && t.tuple) {

            if (tuple->size() != t.tuple->size())
                return true;

            for (size_t i = 0; i < tuple->size(); ++i) {
                if ((*tuple)[i] != (*t.tuple)[i])
                    return true;
            }

            return false;

        } else if (!tuple && !t.tuple) {
            return false;

        } else {
            return true;
        }
    }

    bool operator==(const Type& t) const {
        return !(t != *this);
    }
    
    static std::string print(const Type& t) {
        std::string ret;

        switch (t.type) {
        case NONE: ret += "NONE"; break;
        case ATOM:
            switch (t.atom) {
            case INT: ret += "INT"; break;
            case UINT: ret += "UINT"; break;
            case REAL: ret += "REAL"; break;
            case STRING: ret += "STRING"; break;
            }
            break;
        case TUP: ret += "TUPLE"; break;
        case ARR: ret += "ARRAY"; break;
        case MAP: ret += "MAP"; break;
        case SEQ: ret += "SEQ"; break;
        }

        if (t.tuple) {
            ret += "(";

            for (const Type& tt : *(t.tuple)) {
                ret += " ";
                ret += print(tt);
            }

            ret += " )";
        }

        return ret;
    }

    Type& push(const Type& t) {

        if (!tuple) {
            tuple = std::make_shared< std::vector<Type> >();
        }

        tuple->push_back(t);
        return tuple->back();
    }
};

namespace std {

template <>
struct hash<Type> {
    size_t operator()(const Type& t) const {

        size_t r = hash<size_t>()(t.type);

        if (t.type == Type::ATOM) {
            r += hash<size_t>()(t.atom);

        } else if (t.tuple) {
        
            for (auto i : (*t.tuple)) {
                r += (*this)(i);
            }
        }

        return r;
    }
};

template <>
struct hash< std::pair<String, Type> > {
    size_t operator()(const std::pair<String, Type>& x) const {

        return hash<size_t>()(x.first.ix) + hash<Type>()(x.second);
    }
};

}

#endif
