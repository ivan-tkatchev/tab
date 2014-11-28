#ifndef __TAB_OBJECT_H
#define __TAB_OBJECT_H

namespace obj {

struct Object {

    typedef std::function<Object*(Object*,bool&)> iterator_t;

    virtual ~Object() {}

    virtual size_t hash() const {
        throw std::runtime_error("Object hash not implemented");
    }

    virtual bool eq(Object*) const {
        throw std::runtime_error("Object equality not implemented");
    }

    virtual void print() { }

    virtual Object* clone() const {
        throw std::runtime_error("Object cloning not implemented");
    }

    virtual iterator_t iter() const {
        throw std::runtime_error("Object iteration not implemented");
    }

    virtual void fill(Object*) {
        throw std::runtime_error("Object construction not implemented");
    }

    virtual Object* next(bool&) { throw std::runtime_error("Object 'next' operator not implemented"); }
};

template <typename T>
T& get(const Object* o) {
    return *((T*)o);
}

void __sequencer_print(Object* o) {

    bool ok = true;

    while (ok) {
        o->next(ok)->print();

        if (ok)
            std::cout << std::endl;
    }
}

struct Sequencer : public Object {

    iterator_t v;
    Object* holder;

    iterator_t iter() const {
        return v;
    }
        
    void wrap(Object* i) {
        v = i->iter();
    }

    Object* next(bool& ok) {
        return v(holder, ok);
    }

    void print() {
        __sequencer_print(this);
    }
};

template <typename T>
struct Atom : public Object {
    T v;

    Atom(const T& i = T()) : v(i) {}

    size_t hash() const { return std::hash<T>()(v); }
    bool eq(Object* a) const { return v == get< Atom<T> >(a).v; }
    void print() { std::cout << v; }
    Object* clone() const { return new Atom<T>(v); }

    iterator_t iter() const { return [this](Object* i, bool& ok) { ok = false; return (Object*)this; }; }
};

typedef Atom<::Int> Int;
typedef Atom<::UInt> UInt;
typedef Atom<::Real> Real;
typedef Atom<std::string> String;


template <typename T>
struct ArrayAtom : public Object {
    std::vector<T> v;

    size_t hash() const {
        size_t ret = 0;
        for (const T& t : v) {
            ret += std::hash<T>()(t);
        }
        return ret;
    }

    bool eq(Object* a) const {
        return v == get< ArrayAtom<T> >(a).v;
    }

    void print() {
        bool first = true;
        for (const T& x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << std::endl;
            }

            std::cout << x;
        }
    }

    Object* clone() const {
        ArrayAtom<T>* ret = new ArrayAtom<T>;
        ret->v.assign(v.begin(), v.end());
        return ret;
    }

    void fill(Object* seq) {

        while (1) {

            bool ok;
            obj::Object* next = seq->next(ok);

            v.push_back(get< Atom<T> >(next).v);

            if (!ok) break;
        }
    }

    iterator_t iter() const {

        typename std::vector<T>::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty array");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Atom<T>& x = get< Atom<T> >(i);
            x.v = *ite;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return i;
        };
    }
};

struct ArrayObject : public Object {
    std::vector<Object*> v;

    ~ArrayObject() {
        for (Object* x : v) {
            delete x;
        }
    }
    
    size_t hash() const {
        size_t ret = 0;
        for (Object* t : v) {
            ret += t->hash();
        }
        return ret;
    }

    bool eq(Object* a) const {

        const std::vector<Object*>& b = get<ArrayObject>(a).v;

        if (v.size() != b.size())
            return false;
        
        for (size_t i = 0; i < v.size(); ++i) {
            if (!(v[i]->eq(b[i])))
                return false;
        }

        return true;
    }

    void print() {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << std::endl;
            }

            x->print();
        }
    }

    Object* clone() const {

        ArrayObject* ret = new ArrayObject;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void fill(Object* seq) {

        while (1) {

            bool ok;
            obj::Object* next = seq->next(ok);

            v.push_back(next->clone());

            if (!ok) break;
        }
    }
        
    iterator_t iter() const {

        typename std::vector<Object*>::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty array");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Object* ret = *ite;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return ret;
        };
    }
};

struct Tuple : public ArrayObject {

    void print() {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << "\t";
            }

            x->print();
        }
    }

    template <typename I>
    void set(I b, I e) {
        v.assign(b, e);
    }

    Object* clone() const {

        Tuple* ret = new Tuple;

        for (const Object* s : v) {
            ret->v.push_back(s->clone());
        }

        return ret;
    }

    void fill(Object* s) {
        throw std::runtime_error("Cannot construct tuples");
    }
    
    iterator_t iter() const { return [this](Object* i, bool& ok) { ok = false; return (Object*)this; }; }
};

struct ObjectHash {
    size_t operator()(Object* o) const {
        return o->hash();
    }
};

struct ObjectEq {
    bool operator()(Object* a, Object* b) const {
        return a->eq(b);
    }
};

struct MapObject : public Object {

    typedef std::unordered_map<Object*, Object*, ObjectHash, ObjectEq> map_t;
    map_t v;

    ~MapObject() {
        for (const auto& x : v) {
            delete x.first;
            delete x.second;
        }
    }

    size_t hash() const {
        size_t ret = 0;
        for (const auto& t : v) {
            ret += t.first->hash();
            ret += t.second->hash();
        }
        return ret;
    }

    bool eq(Object* a) const {

        const map_t& b = get<MapObject>(a).v;

        if (v.size() != b.size())
            return false;

        auto i = v.begin();
        auto ie = v.end();
        auto j = b.begin();

        while (i != ie) {

            if (!(i->first->eq(j->first)) ||
                !(i->second->eq(j->second))) {

                return false;
            }
            
            ++i;
            ++j;
        }

        return true;
    }

    void print() {
        bool first = true;
        for (const auto& x : v) {

            if (first) {
                first = false;
            } else {
                std::cout << std::endl;
            }

            x.first->print();
            std::cout << "\t";
            x.second->print();
        }
    }

    Object* clone() const {

        MapObject* ret = new MapObject;

        for (const auto& x : v) {
            Object* k = x.first->clone();
            Object* v = x.second->clone();
            ret->v[k] = v;
        }

        return ret;
    }

    void fill(Object* seq) {

        while (1) {

            bool ok;
            obj::Object* next = seq->next(ok);

            Tuple& tup = get<Tuple>(next);
            obj::Object* key = tup.v[0]->clone();
            obj::Object* val = tup.v[1]->clone();

            auto i  = v.find(key);
            
            if (i != v.end()) {
                delete i->second;
                i->second = val;

            } else {
                v[key] = val;
            }

            if (!ok) break;
        }
    }

    iterator_t iter() const {

        typename map_t::const_iterator ite = v.begin();

        if (ite == v.end())
            throw std::runtime_error("Iterating an empty map");
        
        return [this,ite](Object* i, bool& ok) mutable {

            Tuple& x = get<Tuple>(i);
            x.v[0] = ite->first;
            x.v[1] = ite->second;
            ++ite;

            if (ite == v.end()) {
                ok = false;
                ite = v.begin();
            } else {
                ok = true;
            }

            return i;
        };
    }
};



template <typename... U>
Object* make(const Type& t, U&&... u) {

    if (t.type == Type::NONE)
        return new Object();
    
    if (t.type == Type::ATOM) {
        switch (t.atom) {
        case Type::INT:
            return new Int(std::forward<U>(u)...);
        case Type::UINT:
            return new UInt(std::forward<U>(u)...);
        case Type::REAL:
            return new Real(std::forward<U>(u)...);
        case Type::STRING:
            return new String(std::forward<U>(u)...);
        }
    }

    if (!t.tuple || t.tuple->empty())
        return new Object();

    if (t.type == Type::TUP) {

        Tuple* ret = new Tuple(std::forward<U>(u)...);

        for (const Type& st : (*t.tuple)) {
            ret->v.push_back(make(st, std::forward<U>(u)...));
        }

        return ret;

    } else if (t.type == Type::ARR) {

        const Type& s = (*t.tuple)[0];

        if (s.type == Type::ATOM) {

            switch (s.atom) {
            case Type::INT:
                return new ArrayAtom<::Int>(std::forward<U>(u)...);
            case Type::UINT:
                return new ArrayAtom<::UInt>(std::forward<U>(u)...);
            case Type::REAL:
                return new ArrayAtom<::Real>(std::forward<U>(u)...);
            case Type::STRING:
                return new ArrayAtom<std::string>(std::forward<U>(u)...);
            }
        }

        return new ArrayObject(std::forward<U>(u)...);

    } else if (t.type == Type::MAP) {

        return new MapObject(std::forward<U>(u)...);

    } else if (t.type == Type::SEQ) {

        Sequencer* ret = new Sequencer;

        ret->holder = make((*t.tuple)[0]);

        return ret;
    }

    throw std::runtime_error("Sanity error: cannot create object");
}


struct SequencerFlatten : public Object {

    Object* holder;
    bool subseq_ok;
    Object* subseq;
    iterator_t subi;
    
    SequencerFlatten(const Type& t) {

        holder = make((*t.tuple)[0]);
    }

    void wrap(Object* s) {

        subseq = s;
        subi = subseq->next(subseq_ok)->iter();
    }

    Object* next(bool& ok) {

        bool sok;
        Object* ret = subi(holder, sok);

        if (!sok && !subseq_ok) {
            ok = false;

        } else if (!sok) {
            subi = subseq->next(subseq_ok)->iter();
            ok = true;

        } else {
            ok = true;
        }

        return ret;
    }

    iterator_t iter() const {
        return [this](Object* holder, bool& ok) mutable { return next(ok); };
    }

    void print() {
        __sequencer_print(this);
    }
};


}

#endif
