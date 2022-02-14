#ifndef __TAB_TYPE_H
#define __TAB_TYPE_H

namespace tab {
   
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
    std::shared_ptr< Atom > literal;
    
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
        case NONE: ret += "None"; break;
        case ATOM:
            switch (t.atom) {
            case INT: ret += "Int"; break;
            case UINT: ret += "UInt"; break;
            case REAL: ret += "Real"; break;
            case STRING: ret += "String"; break;
            }
            break;
        case TUP: ret += "("; break;
        case ARR: ret += "Arr["; break;
        case MAP: ret += "Map["; break;
        case SEQ: ret += "Seq["; break;
        }

        if (t.tuple) {
            bool first = true;

            for (const Type& tt : *(t.tuple)) {
                if (first) {
                    first = false;
                } else {
                    ret += ", ";
                }
                ret += print(tt);
            }

            switch (t.type) {
            case TUP:
                ret += ")"; break;
            case ARR:
            case MAP:
            case SEQ:
                ret += "]"; break;
            default:
                break;
            }
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

    void pop_front() {
        if (tuple && tuple->size() > 1) {
            tuple = std::make_shared< std::vector<Type> >(tuple->begin()+1, tuple->end());
        }
    }
};

} // namespace tab

namespace std {

template <>
struct hash<tab::Type> {
    size_t operator()(const tab::Type& t) const {

        size_t r = hash<size_t>()(t.type);

        if (t.type == tab::Type::ATOM) {
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
struct hash< std::pair<tab::String, tab::Type> > {
    size_t operator()(const std::pair<tab::String, tab::Type>& x) const {

        return hash<size_t>()(x.first.ix) + hash<tab::Type>()(x.second);
    }
};

} // namespace std

#endif
