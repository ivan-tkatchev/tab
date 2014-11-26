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


void cut(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& del = obj::get<obj::String>(args.v[1]).v;

    std::cout << "cut '" << str << "' '" << del << "'" << std::endl;
    
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

void grep(const obj::Object* in, obj::Object*& out) {

    static std::unordered_map<std::string, std::regex> _cache;

    
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    auto i = _cache.find(regex);

    if (i == _cache.end()) {
        i = _cache.insert(i, std::make_pair(regex, std::regex(regex, std::regex_constants::optimize)));
    }

    std::regex& r = i->second;

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
              funcs::cut);

    funcs.add("grep",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              funcs::grep);
}

#endif
