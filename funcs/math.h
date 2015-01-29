#ifndef __TAB_FUNCS_MATH_H
#define __TAB_FUNCS_MATH_H

template <typename X, typename Y>
void x_to_y(const obj::Object* in, obj::Object*& out) {
    obj::get<Y>(out).v = obj::get<X>(in).v;
}

void string_to_real(const obj::Object* in, obj::Object*& out) {
    try {
        obj::get<obj::Real>(out).v = std::stod(obj::get<obj::String>(in).v);
    } catch (std::exception& e) {
        throw std::runtime_error("Could not convert '" + obj::get<obj::String>(in).v + "' to a floating-point number.");
    }
}

void string_to_int(const obj::Object* in, obj::Object*& out) {
    try {
        obj::get<obj::Int>(out).v = std::stol(obj::get<obj::String>(in).v);
    } catch (std::exception& e) {
        throw std::runtime_error("Could not convert '" + obj::get<obj::String>(in).v + "' to an integer.");
    }
}

void string_to_uint(const obj::Object* in, obj::Object*& out) {
    try {
        obj::get<obj::UInt>(out).v = std::stoul(obj::get<obj::String>(in).v);
    } catch (std::exception& e) {
        throw std::runtime_error("Could not convert '" + obj::get<obj::String>(in).v + "' to an integer.");
    }
}

template <typename T>
void to_string(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::String>(out).v = std::to_string(obj::get<T>(in).v);
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

void round(const obj::Object* in, obj::Object*& out) {
    obj::get<obj::Real>(out).v = ::round(obj::get<obj::Real>(in).v);
}

template <typename T>
void abs(const obj::Object* in, obj::Object*& out) {
    obj::get<T>(out).v = std::abs(obj::get<T>(in).v);
}

void register_math(Functions& funcs) {

    funcs.add("real", Type(Type::INT), Type(Type::REAL), x_to_y<obj::Int,obj::Real>);
    funcs.add("real", Type(Type::UINT), Type(Type::REAL), x_to_y<obj::UInt,obj::Real>);
    funcs.add("real", Type(Type::STRING), Type(Type::REAL), string_to_real);

    funcs.add("int", Type(Type::UINT), Type(Type::INT), x_to_y<obj::UInt,obj::Int>);
    funcs.add("int", Type(Type::REAL), Type(Type::INT), x_to_y<obj::Real,obj::Int>);
    funcs.add("int", Type(Type::STRING), Type(Type::INT), string_to_int);

    funcs.add("uint", Type(Type::INT), Type(Type::UINT), x_to_y<obj::Int,obj::UInt>);
    funcs.add("uint", Type(Type::REAL), Type(Type::UINT), x_to_y<obj::Real,obj::UInt>);
    funcs.add("uint", Type(Type::STRING), Type(Type::UINT), string_to_uint);

    funcs.add("string", Type(Type::INT), Type(Type::STRING), to_string<obj::Int>);
    funcs.add("string", Type(Type::UINT), Type(Type::STRING), to_string<obj::UInt>);
    funcs.add("string", Type(Type::REAL), Type(Type::STRING), to_string<obj::Real>);

    funcs.add("pi", Type(), Type(Type::REAL), pi);
    funcs.add("e", Type(), Type(Type::REAL), e);

    funcs.add("exp", Type(Type::INT), Type(Type::REAL), exp<obj::Int>);
    funcs.add("exp", Type(Type::UINT), Type(Type::REAL), exp<obj::UInt>);
    funcs.add("exp", Type(Type::REAL), Type(Type::REAL), exp<obj::Real>);

    funcs.add("sqrt", Type(Type::INT), Type(Type::REAL), sqrt<obj::Int>);
    funcs.add("sqrt", Type(Type::UINT), Type(Type::REAL), sqrt<obj::UInt>);
    funcs.add("sqrt", Type(Type::REAL), Type(Type::REAL), sqrt<obj::Real>);

    funcs.add("log", Type(Type::INT), Type(Type::REAL), log<obj::Int>);
    funcs.add("log", Type(Type::UINT), Type(Type::REAL), log<obj::UInt>);
    funcs.add("log", Type(Type::REAL), Type(Type::REAL), log<obj::Real>);

    funcs.add("sin", Type(Type::INT), Type(Type::REAL), sin<obj::Int>);
    funcs.add("sin", Type(Type::UINT), Type(Type::REAL), sin<obj::UInt>);
    funcs.add("sin", Type(Type::REAL), Type(Type::REAL), sin<obj::Real>);

    funcs.add("cos", Type(Type::INT), Type(Type::REAL), cos<obj::Int>);
    funcs.add("cos", Type(Type::UINT), Type(Type::REAL), cos<obj::UInt>);
    funcs.add("cos", Type(Type::REAL), Type(Type::REAL), cos<obj::Real>);

    funcs.add("tan", Type(Type::INT), Type(Type::REAL), tan<obj::Int>);
    funcs.add("tan", Type(Type::UINT), Type(Type::REAL), tan<obj::UInt>);
    funcs.add("tan", Type(Type::REAL), Type(Type::REAL), tan<obj::Real>);

    funcs.add("round", Type(Type::REAL), Type(Type::REAL), round);

    funcs.add("abs", Type(Type::REAL), Type(Type::REAL), abs<obj::Real>);
    funcs.add("abs", Type(Type::INT), Type(Type::INT), abs<obj::Int>);
}

#endif

