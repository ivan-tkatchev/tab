#ifndef __TAB_FUNCS_H
#define __TAB_FUNCS_H

namespace funcs {

template <typename X, typename Y>
void x_to_y(const obj::Object* in, obj::Object*& out) {
    obj::get<Y>(out).v = obj::get<X>(in).v;
}

void string_to_real(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = std::stod(obj::get<obj::String>(in).v);
}

void string_to_int(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Int>(out).v = std::stol(obj::get<obj::String>(in).v);
}

void string_to_uint(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::UInt>(out).v = std::stoul(obj::get<obj::String>(in).v);
}

void pi(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = M_PI;
}

void e(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = M_E;
}

template <typename T>
void exp(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::exp(obj::get<T>(in).v);
}

template <typename T>
void sqrt(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::sqrt(obj::get<T>(in).v);
}

template <typename T>
void log(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::log(obj::get<T>(in).v);
}

template <typename T>
void sin(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::sin(obj::get<T>(in).v);
}

template <typename T>
void cos(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::cos(obj::get<T>(in).v);
}

template <typename T>
void tan(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::tan(obj::get<T>(in).v);
}

void cutn(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& del = obj::get<obj::String>(args.v[1]).v;

    size_t N = str.size();
    size_t M = del.size();

    size_t prev = 0;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;
    
    v.clear();
    
    for (size_t i = 0; i < N; ++i) {

        bool matched = true;

        for (size_t j = 0; j < M; ++j) {

            if (i+j < N && str[i+j] == del[j])
                continue;
            
            matched = false;
            break;
        }

        if (matched) {
            v.emplace_back(str.begin() + prev, str.begin() + i);
            i += M;
            prev = i;
            --i;
        }
    }

    v.emplace_back(str.begin() + prev, str.end());
}

void cut1(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& del = obj::get<obj::String>(args.v[1]).v;

    size_t N = str.size();
    unsigned char d = del.c_str()[0];

    size_t prev = 0;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;
    
    v.clear();

    for (size_t i = 0; i < N; ++i) {

        if (str[i] == d) {
            v.emplace_back(str.begin() + prev, str.begin() + i);
            prev = i+1;
        }
    }

    v.emplace_back(str.begin() + prev, str.end());
}

void grep(const obj::Object* in, obj::Object*& out) {

    static std::unordered_map<std::string, std::regex> _cache;

    
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    std::cout << "!! (" << str << ") [" << regex << "]" << std::endl;
    
    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    auto i = _cache.find(regex);

    if (i == _cache.end()) {
        i = _cache.insert(i, std::make_pair(regex, std::regex(regex, std::regex_constants::optimize)));
    }

    const std::regex& r = i->second;

    std::sregex_iterator iter(str.begin(), str.end(), r);
    std::sregex_iterator end;

    while (iter != end) {

        if (iter->size() == 1) {

            v.emplace_back(iter->str());

        } else if (iter->size() > 1) {
            auto subi = iter->begin();
            auto sube = iter->end();
            ++subi;
            
            while (subi != sube) {
                v.emplace_back(subi->str());
                ++subi;
            }
        }

        ++iter;
    }

    if (v.empty())
        v.emplace_back();
}

void count_seq(const obj::Object* in, obj::Object*& out) {

    obj::Sequencer& seq = obj::get<obj::Sequencer>(in);
    UInt& i = obj::get<obj::UInt>(out).v;
    
    static bm_scope bms("count(seq)");
    bm __(bms);

    i = 0;
    bool ok = true;

    while (ok) {
        seq.next(ok);
        ++i;
    }
}

template <typename T>
void count_arratom(const obj::Object* in, obj::Object*& out) {

    const auto& arr = obj::get< obj::ArrayAtom<T> >(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = arr.v.size();
}

void count_arr(const obj::Object* in, obj::Object*& out) {

    const auto& arr = obj::get<obj::ArrayObject>(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = arr.v.size();
}

void count_map(const obj::Object* in, obj::Object*& out) {

    const auto& map = obj::get<obj::MapObject>(in);
    UInt& i = obj::get<obj::UInt>(out).v;

    i = map.v.size();
}

Functions::func_t count_checker(const Type& args, Type& ret, obj::Object*&) {

    ret = Type(Type::UINT);

    switch (args.type) {
    case Type::SEQ:
        return count_seq;
    case Type::MAP:
        return count_map;
    case Type::ARR:
        switch (args.tuple->at(0).type) {
        case Type::INT:
            return count_arratom<Int>;
        case Type::UINT:
            return count_arratom<UInt>;
        case Type::REAL:
            return count_arratom<Real>;
        case Type::STRING:
            return count_arratom<std::string>;
        default:
            return count_arr;
        }
    default:
        return nullptr;
    }
}

struct SequencerHeadSeq : public obj::Object {

    obj::Object* seq;
    UInt n;
    UInt i;

    SequencerHeadSeq() : n(0), i(0) {}

    void wrap(obj::Object* s) {
        seq = s;
    }
    
    obj::Object* next(bool& ok) {

        obj::Object* ret = seq->next(ok);
        ++i;

        if (i >= n)
            ok = false;

        return ret;
    }

    iterator_t iter() const {
        return [this](obj::Object* o, bool& ok) mutable { return next(ok); };
    }
        
};

struct SequencerHeadVal : public obj::Object {

    obj::Object* holder;
    iterator_t v;
    UInt n;
    UInt i;

    SequencerHeadVal(const Type& t) : n(0), i(0) {
        holder = obj::make(t);
    }

    void wrap(obj::Object* s) {
        v = s->iter();
    }
    
    obj::Object* next(bool& ok) {

        obj::Object* ret = v(holder, ok);
        ++i;

        if (i >= n)
            ok = false;

        return ret;
    }

    iterator_t iter() const {
        return v;
    }
};

template <typename T>
void head(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& inp = obj::get<obj::Tuple>(in);
    obj::Object* arg = inp.v[0];
    UInt n = obj::get<obj::UInt>(inp.v[1]).v;

    T& seq = obj::get<T>(out);

    seq.n = n;
    seq.i = 0;
    seq.wrap(arg);
}

Functions::func_t head_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& a2 = args.tuple->at(1);

    if (!check_integer(a2))
        return nullptr;

    const Type& a1 = args.tuple->at(0);
    ret = wrap_seq(a1);

    if (a1.type == Type::SEQ) {

        obj = new SequencerHeadSeq;
        return head<SequencerHeadSeq>;
    }
    
    obj = new SequencerHeadVal(a1.tuple->at(0));
    return head<SequencerHeadVal>;
}

}

void register_functions() {

    Functions& funcs = functions_init();

    funcs.add("real", Type(Type::INT), Type(Type::REAL), funcs::x_to_y<obj::Int,obj::Real>);
    funcs.add("real", Type(Type::UINT), Type(Type::REAL), funcs::x_to_y<obj::UInt,obj::Real>);
    funcs.add("real", Type(Type::STRING), Type(Type::REAL), funcs::string_to_real);

    funcs.add("int", Type(Type::UINT), Type(Type::INT), funcs::x_to_y<obj::UInt,obj::Int>);
    funcs.add("int", Type(Type::REAL), Type(Type::INT), funcs::x_to_y<obj::Real,obj::Int>);
    funcs.add("int", Type(Type::STRING), Type(Type::INT), funcs::string_to_int);

    funcs.add("uint", Type(Type::INT), Type(Type::UINT), funcs::x_to_y<obj::Int,obj::UInt>);
    funcs.add("uint", Type(Type::REAL), Type(Type::UINT), funcs::x_to_y<obj::Real,obj::UInt>);
    funcs.add("uint", Type(Type::STRING), Type(Type::UINT), funcs::string_to_uint);

    funcs.add("pi", Type(), Type(Type::REAL), funcs::pi);
    funcs.add("e", Type(), Type(Type::REAL), funcs::e);

    funcs.add("exp", Type(Type::INT), Type(Type::REAL), funcs::exp<obj::Int>);
    funcs.add("exp", Type(Type::UINT), Type(Type::REAL), funcs::exp<obj::UInt>);
    funcs.add("exp", Type(Type::REAL), Type(Type::REAL), funcs::exp<obj::Real>);

    funcs.add("sqrt", Type(Type::INT), Type(Type::REAL), funcs::sqrt<obj::Int>);
    funcs.add("sqrt", Type(Type::UINT), Type(Type::REAL), funcs::sqrt<obj::UInt>);
    funcs.add("sqrt", Type(Type::REAL), Type(Type::REAL), funcs::sqrt<obj::Real>);

    funcs.add("log", Type(Type::INT), Type(Type::REAL), funcs::log<obj::Int>);
    funcs.add("log", Type(Type::UINT), Type(Type::REAL), funcs::log<obj::UInt>);
    funcs.add("log", Type(Type::REAL), Type(Type::REAL), funcs::log<obj::Real>);

    funcs.add("sin", Type(Type::INT), Type(Type::REAL), funcs::sin<obj::Int>);
    funcs.add("sin", Type(Type::UINT), Type(Type::REAL), funcs::sin<obj::UInt>);
    funcs.add("sin", Type(Type::REAL), Type(Type::REAL), funcs::sin<obj::Real>);

    funcs.add("cos", Type(Type::INT), Type(Type::REAL), funcs::cos<obj::Int>);
    funcs.add("cos", Type(Type::UINT), Type(Type::REAL), funcs::cos<obj::UInt>);
    funcs.add("cos", Type(Type::REAL), Type(Type::REAL), funcs::cos<obj::Real>);

    funcs.add("tan", Type(Type::INT), Type(Type::REAL), funcs::tan<obj::Int>);
    funcs.add("tan", Type(Type::UINT), Type(Type::REAL), funcs::tan<obj::UInt>);
    funcs.add("tan", Type(Type::REAL), Type(Type::REAL), funcs::tan<obj::Real>);

    funcs.add("cut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::cut1);
    
    funcs.add("cutn",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::cutn);

    funcs.add("grep",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::grep);

    funcs.add_poly("count", funcs::count_checker);

    funcs.add_poly("head", funcs::head_checker);
}

#endif
