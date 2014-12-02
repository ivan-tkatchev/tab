#ifndef __TAB_OBJECT_H
#define __TAB_OBJECT_H

namespace obj {

struct Object {

    virtual ~Object() {}

    virtual size_t hash() const {
        throw std::runtime_error("Object hash not implemented");
    }

    virtual bool eq(Object*) const {
        throw std::runtime_error("Object equality not implemented");
    }

    virtual bool less(Object*) const {
        throw std::runtime_error("Object sorting not implemented");
    }

    virtual void print() { }

    virtual Object* clone() const {
        throw std::runtime_error("Object cloning not implemented");
    }

    virtual void fill(Object*) {
        throw std::runtime_error("Object construction not implemented");
    }

    virtual void wrap(Object*) { throw std::runtime_error("Object sequence wrapping is not implemented"); }
    
    virtual Object* next() { throw std::runtime_error("Object 'next' operator not implemented"); }

    virtual void merge_start() {}
    virtual void merge(const Object*) {}
    virtual void merge_end() {}
};

template <typename T>
T& get(const Object* o) {
    return *((T*)o);
}

Object* nothing() {
    static Object* ret = new Object;
    return ret;
}

template <typename T>
struct Atom : public Object {
    T v;

    Atom(const T& i = T()) : v(i) {}

    size_t hash() const { return std::hash<T>()(v); }
    bool eq(Object* a) const { return v == get< Atom<T> >(a).v; }
    bool less(Object* a) const { return v < get< Atom<T> >(a).v; }
    void print() { std::cout << v; }
    Object* clone() const { return new Atom<T>(v); }
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

    bool less(Object* a) const {
        return v < get< ArrayAtom<T> >(a).v;
    }
    
    void print() {
        for (const T& x : v) {
            std::cout << x << std::endl;
        }
    }

    Object* clone() const {
        ArrayAtom<T>* ret = new ArrayAtom<T>;
        ret->v.assign(v.begin(), v.end());
        return ret;
    }

    void fill(Object* seq) {

        v.clear();

        while (1) {

            Object* next = seq->next();

            if (!next) break;
            
            v.push_back(get< Atom<T> >(next).v);
        }
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

    bool less(Object* a) const {
        const std::vector<Object*>& other = get<ArrayObject>(a).v;

        auto ai = v.begin();
        auto ae = v.end();
        auto bi = other.begin();
        auto be = other.end();

        while (1) {

            if (ai == ae || bi == be)
                return (bi != be);

            if ((*ai)->less(*bi))
                return true;

            if ((*bi)->less(*ai))
                return false;
            
            ++ai;
            ++bi;
        }

        return false;
    }

    void print() {
        for (Object* x : v) {

            x->print();
            std::cout << std::endl;
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

        v.clear();

        while (1) {

            Object* next = seq->next();

            if (!next) break;
            
            v.push_back(next->clone());
        }
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

    virtual void merge_start() {
        for (Object* o : v) {
            o->merge_start();
        }
    }
    
    virtual void merge(const Object* v2) {
        Tuple& t = get<Tuple>(v2);

        for (size_t i = 0; i < v.size(); ++i) {
            v[i]->merge(t.v[i]);
        }
    }

    virtual void merge_end() {
        for (Object* o : v) {
            o->merge_end();
        }
    }
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

struct ObjectLess {
    bool operator()(Object* a, Object* b) const {
        return a->less(b);
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

    bool less(Object* a) const {
        const map_t& other = get<MapObject>(a).v;

        auto ai = v.begin();
        auto ae = v.end();
        auto bi = other.begin();
        auto be = other.end();

        while (1) {

            if (ai == ae || bi == be)
                return (bi != be);

            if (ai->first->less(bi->first))
                return true;

            if (bi->first->less(ai->first))
                return false;

            if (ai->second->less(bi->second))
                return true;

            if (bi->second->less(ai->second))
                return false;

            ++ai;
            ++bi;
        }

        return false;
    }

    void print() {
        for (const auto& x : v) {

            x.first->print();
            std::cout << "\t";
            x.second->print();
            std::cout << std::endl;
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

        v.clear();

        while (1) {

            Object* next = seq->next();

            if (!next) break;

            Tuple& tup = get<Tuple>(next);

            Object* key = tup.v[0];
            Object* val = tup.v[1];

            auto i  = v.find(key);
            
            if (i != v.end()) {
                i->second->merge(val);

            } else {
                key = key->clone();
                val = val->clone();
                val->merge_start();
                v[key] = val;
            }
        }

        for (auto& i : v) {
            i.second->merge_end();
        }
    }
};


struct SeqBase : public Object {

    Object* clone() const {
        throw std::runtime_error("Sequences cannot be stored in arrays and maps.");
    }

    void print() {

        while (1) {

            Object* v = this->next();

            if (!v) break;

            v->print();
            std::cout << std::endl;
        }
    }
};

struct SeqSingle : public SeqBase {

    Object* atom;
    bool toggle;
    
    void wrap(Object* v) {
        atom = v;
        toggle = false;
    }

    Object* next() {
        if (toggle) return nullptr;

        toggle = !toggle;
        return atom;
    }
};

template <typename T>
struct SeqArrayAtom : public SeqBase {

    ArrayAtom<T>* arr;
    Atom<T>* holder;
    typename std::vector<T>::const_iterator b;
    typename std::vector<T>::const_iterator e;

    SeqArrayAtom() {
        holder = new Atom<T>;
    }
    
    void wrap(Object* a) {
        arr = (ArrayAtom<T>*)a;
        b = arr->v.begin();
        e = arr->v.end();
    }

    Object* next() {

        if (b == e) {
            return nullptr;
        }

        holder->v = *b;
        ++b;

        return holder;
    }
};

struct SeqArrayObject : public SeqBase {

    ArrayObject* arr;
    typename std::vector<Object*>::const_iterator b;
    typename std::vector<Object*>::const_iterator e;

    void wrap(Object* a) {
        arr = (ArrayObject*)a;
        b = arr->v.begin();
        e = arr->v.end();
    }

    Object* next() {

        if (b == e) {
            return nullptr;
        }

        Object* ret = *b;
        ++b;

        return ret;
    }
};

struct SeqMapObject : public SeqBase {

    MapObject* map;
    Tuple* holder;
    typename MapObject::map_t::const_iterator b;
    typename MapObject::map_t::const_iterator e;

    SeqMapObject() {
        holder = new Tuple;
        holder->v.resize(2);
    }
    
    void wrap(Object* a) {
        map = (MapObject*)a;
        b = map->v.begin();
        e = map->v.end();
    }

    Object* next() {

        if (b == e) {
            return nullptr;
        }

        holder->v[0] = b->first;
        holder->v[1] = b->second;
        ++b;

        return holder;
    }
};

struct SeqGenerator : public SeqBase {

    typedef std::function<Object*()> iterator_t;
    iterator_t v;

    void wrap(Object* o) {
        throw std::runtime_error("Sanity error: sequence wrapping a generator.");
    }
    
    Object* next() {
        return v();
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

        //throw std::runtime_error("Sanity error: cannot construct sequences.");
        return nullptr;
    }

    throw std::runtime_error("Sanity error: cannot create object");
}

Object* make_seq_from(const Type& s) {

    if (s.type == Type::ATOM || s.type == Type::TUP) {

        return new SeqSingle;

    } else if (s.type == Type::MAP) {

        return new SeqMapObject;

    } else if (s.type == Type::ARR) {

        const Type& s2 = (*s.tuple)[0];

        if (s2.type == Type::ATOM) {

            switch (s2.atom) {
            case Type::INT:
                return new SeqArrayAtom<::Int>;
            case Type::UINT:
                return new SeqArrayAtom<::UInt>;
            case Type::REAL:
                return new SeqArrayAtom<::Real>;
            case Type::STRING:
                return new SeqArrayAtom<std::string>;
            }

        } else {

            return new SeqArrayObject;
        }

    } else if (s.type == Type::SEQ) {

        return nullptr;
    }

    throw std::runtime_error("Cannot construct a sequence from a " + Type::print(s));
}

}

#endif
