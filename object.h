#ifndef __TAB_OBJECT_H
#define __TAB_OBJECT_H

namespace tab {

namespace obj {

struct Printer {

    virtual void val(tab::UInt v) { printf("%lu", v); }
    virtual void val(tab::Int v)  { printf("%ld", v); }
    virtual void val(tab::Real v) { printf("%g", v); }

    virtual void val(const std::string& v) {
        fwrite(v.data(), sizeof(char), v.size(), stdout);
    }

    virtual void rs() { printf("\t"); }
    virtual void nl() { printf("\n"); }
    virtual void alts() { printf(";"); }
};

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

    virtual void print(Printer&) { }

    virtual Object* clone() const {
        throw std::runtime_error("Object cloning not implemented");
    }

    virtual void fill(Object*) {
        throw std::runtime_error("Object construction not implemented");
    }

    virtual void wrap(Object*) { throw std::runtime_error("Object sequence wrapping is not implemented"); }
    
    virtual Object* next() { throw std::runtime_error("Object 'next' operator not implemented"); }

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
    void print(Printer& p) { p.val(v); }
    Object* clone() const { return new Atom<T>(v); }
};

typedef Atom<tab::Int> Int;
typedef Atom<tab::UInt> UInt;
typedef Atom<tab::Real> Real;
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
    
    void print(Printer& p) {
        bool first = true;

        for (const T& x : v) {
            if (first) {
                first = false;
            } else {
                p.nl();
            }

            p.val(x);
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

    void merge(const Object* v2) {
        ArrayAtom<T>& t = get< ArrayAtom<T> >(v2);

        v.insert(v.end(), t.v.begin(), t.v.end());
    }
};

struct ArrayObject : public Object {
    std::vector<Object*> v;

    ~ArrayObject() {
        clear();
    }

    void clear() {

        for (Object* x : v) {
            delete x;
        }

        v.clear();
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

    void print(Printer& p) {
        bool first = true;

        for (Object* x : v) {
            if (first) {
                first = false;
            } else {
                p.nl();
            }

            x->print(p);
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

        clear();

        while (1) {

            Object* next = seq->next();

            if (!next) break;
            
            v.push_back(next->clone());
        }
    }        

    void merge(const Object* v2) {
        ArrayObject& t = get<ArrayObject>(v2);

        for (const Object* s : t.v) {
            v.push_back(s->clone());
        }
    }
};

struct Tuple : public ArrayObject {

    void print(Printer& p) {
        bool first = true;
        for (Object* x : v) {

            if (first) {
                first = false;
            } else {
                p.rs();
            }

            x->print(p);
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

template <bool> struct _map_t;

template <> struct _map_t<true> {
    typedef std::map<Object*, Object*, ObjectLess> type_t;
};

template <> struct _map_t<false> {
    typedef std::unordered_map<Object*, Object*, ObjectHash, ObjectEq> type_t;
};


template <bool SORTED>
struct MapObject : public Object {

    typedef typename _map_t<SORTED>::type_t map_t;
    map_t v;

    ~MapObject() {
        clear();
    }

    void clear() {

        for (const auto& x : v) {
            delete x.first;
            delete x.second;
        }

        v.clear();
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

        const map_t& b = get< MapObject<SORTED> >(a).v;

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
        const map_t& other = get< MapObject<SORTED> >(a).v;

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

    void print(Printer& p) {
        bool first = true;
        for (const auto& x : v) {
            if (first) {
                first = false;
            } else {
                p.nl();
            }

            x.first->print(p);
            p.rs();
            x.second->print(p);
        }
    }

    Object* clone() const {

        MapObject<SORTED>* ret = new MapObject<SORTED>;

        for (const auto& x : v) {
            Object* k = x.first->clone();
            Object* v = x.second->clone();
            ret->v[k] = v;
        }

        return ret;
    }

    void insert(Object* key, Object* val) {

        auto i = v.find(key);
            
        if (i != v.end()) {
            i->second->merge(val);

        } else {
            key = key->clone();
            val = val->clone();
            v[key] = val;
        }
    }

    void fill(Object* seq) {

        clear();

        while (1) {

            Object* next = seq->next();

            if (!next) break;

            Tuple& tup = get<Tuple>(next);

            Object* key = tup.v[0];
            Object* val = tup.v[1];

            insert(key, val);
        }

        for (auto& i : v) {
            i.second->merge_end();
        }
    }

    void merge(const Object* v2) {
        MapObject<SORTED>& t = get< MapObject<SORTED> >(v2);

        for (const auto& i : t.v) {
            Object* key = (Object*)i.first;
            Object* val = (Object*)i.second;

            insert(key, val);
        }
    }

    void merge_end() {
        for (auto& i : v) {
            i.second->merge_end();
        }
    }
};


struct SeqBase : public Object {

    size_t hash() const {
        throw std::runtime_error("Sequences cannot be stored in maps.");
    }

    virtual bool eq(Object*) const {
        throw std::runtime_error("Sequences cannot be compared.");
    }

    virtual bool less(Object*) const {
        throw std::runtime_error("Sequences cannot be compared.");
    }

    Object* clone() const {
        throw std::runtime_error("Sequences cannot be stored in arrays and maps or remembered.");
    }

    void print(Printer& p) {

        bool first = true;
        
        while (1) {

            Object* v = this->next();

            if (!v) break;

            if (first) {
                first = false;
            } else {
                p.nl();
            }

            v->print(p);
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

template <bool SORTED>
struct SeqMapObject : public SeqBase {

    Tuple* holder;
    typename MapObject<SORTED>::map_t::const_iterator b;
    typename MapObject<SORTED>::map_t::const_iterator e;

    SeqMapObject() {
        holder = new Tuple;
        holder->v.resize(2);
    }
    
    void wrap(Object* a) {
        MapObject<SORTED>* map = (MapObject<SORTED>*)a;
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


template <bool SORTED, typename... U>
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
            ret->v.push_back(make<SORTED>(st, std::forward<U>(u)...));
        }

        return ret;

    } else if (t.type == Type::ARR) {

        const Type& s = (*t.tuple)[0];

        if (s.type == Type::ATOM) {

            switch (s.atom) {
            case Type::INT:
                return new ArrayAtom<tab::Int>(std::forward<U>(u)...);
            case Type::UINT:
                return new ArrayAtom<tab::UInt>(std::forward<U>(u)...);
            case Type::REAL:
                return new ArrayAtom<tab::Real>(std::forward<U>(u)...);
            case Type::STRING:
                return new ArrayAtom<std::string>(std::forward<U>(u)...);
            }
        }

        return new ArrayObject(std::forward<U>(u)...);

    } else if (t.type == Type::MAP) {

        return new MapObject<SORTED>(std::forward<U>(u)...);

    } else if (t.type == Type::SEQ) {

        //throw std::runtime_error("Sanity error: cannot construct sequences.");
        return nullptr;
    }

    throw std::runtime_error("Sanity error: cannot create object");
}

template <bool SORTED>
Object* make_seq_from(const Type& s) {

    if (s.type == Type::ATOM || s.type == Type::TUP) {

        return new SeqSingle;

    } else if (s.type == Type::MAP) {

        return new SeqMapObject<SORTED>;

    } else if (s.type == Type::ARR) {

        const Type& s2 = (*s.tuple)[0];

        if (s2.type == Type::ATOM) {

            switch (s2.atom) {
            case Type::INT:
                return new SeqArrayAtom<tab::Int>;
            case Type::UINT:
                return new SeqArrayAtom<tab::UInt>;
            case Type::REAL:
                return new SeqArrayAtom<tab::Real>;
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

} // namespace obj

} // namespace tab

#endif
