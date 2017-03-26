#ifndef __TUP_FUNCS_URL_H
#define __TUP_FUNCS_URL_H


template <typename I>
bool hex_digit_decode(I& i, I e, unsigned char& out) {

    ++i;
    if (i == e)
        return false;

    out = *i;

    if (out >= 'A' && out <= 'F') {
        out = 10 + (out - 'A');

    } else if (out >= 'a' && out <= 'f') {
        out = 10 + (out - 'a');

    } else if (out >= '0' && out <= '9') {
        out = out - '0';
    }

    return true;
}

template <typename I>
void hex_decode(I& i, I e, std::string& out) {

    if (i == e)
        return;

    switch (*i) {
    case '+':
        out += ' ';
        break;
    case '%':
    {
        unsigned char a;
        if (!hex_digit_decode(i, e, a))
            return;

        unsigned char b;
        if (!hex_digit_decode(i, e, b))
            return;

        out += ((a << 4) | b);
        break;
    }
    default:
        out += *i;
    }
}

void url_getparam(const std::string& a, const std::string& param, std::string& out) {

    enum {
        KEY,
        VAL,
        MYVAL
    } state = KEY;

    std::string key;

    auto i = a.begin();
    auto e = a.end();

    while (i != e) {
        unsigned char c = *i;

        if (state == KEY) {

            if (c == '=') {

                if (key == param) {
                    state = MYVAL;
                } else {
                    state = VAL;
                }

                key.clear();

            } else if (c == '?') {
                key.clear();

            } else {
                key += c;
            }

        } else if (state == MYVAL) {

            if (c == '&' || c == ' ') {
                return;
            }

            hex_decode(i, e, out);

        } else {

            if (c == '&') {
                state = KEY;
            } else if (c == ' ') {
                return;
            }
        }

        ++i;
    }
}

void url_getparam(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& url = obj::get<obj::String>(args.v[0]).v;
    const std::string& key = obj::get<obj::String>(args.v[1]).v;

    std::string& val = obj::get<obj::String>(out).v;

    val.clear();

    url_getparam(url, key, val);
}

struct UrlGetter {

    std::string::const_iterator i;
    std::string::const_iterator e;

    enum {
        KEY,
        VAL
    } state;

    void set_url(const std::string& str) {
        i = str.begin();
        e = str.end();
    }

    bool next(std::string& key, std::string& val) {

        if (i == e)
            return false;

        state = KEY;
        key.clear();
        val.clear();

        while (i != e) {
            unsigned char c = *i;

            if (state == KEY) {

                if (c == '=') {
                    state = VAL;

                } else if (c == '?') {
                    key.clear();

                } else {
                    key += c;
                }

            } else if (state == VAL) {

                if (c == '&' || c == ' ') {
                    ++i;
                    return true;
                }

                hex_decode(i, e, val);
            }

            ++i;
        }

        return true;
    }
};

struct SeqUrlGetter : public obj::SeqBase {

    obj::Tuple* holder;

    UrlGetter urlgetter;

    SeqUrlGetter() {
        holder = new obj::Tuple;
        holder->v.push_back(new obj::String);
        holder->v.push_back(new obj::String);
    }

    ~SeqUrlGetter() {
        delete holder;
    }

    void set_url(const std::string& u) {
        urlgetter.set_url(u);
    }

    obj::Object* next() {

        std::string& key = obj::get<obj::String>(holder->v[0]).v;
        std::string& val = obj::get<obj::String>(holder->v[1]).v;

        if (!urlgetter.next(key, val)) {
            return nullptr;
        }

        return holder;
    }
};

void url_getparam_seq(const obj::Object* in, obj::Object*& out) {

    const std::string& url = obj::get<obj::String>(in).v;

    SeqUrlGetter& seq = obj::get<SeqUrlGetter>(out);

    seq.set_url(url);
}

Functions::func_t url_getparam_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args == Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) })) {

        ret = Type(Type::STRING);
        return url_getparam;
    }

    if (args == Type(Type::STRING)) {

        ret = Type(Type::SEQ, { Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) })});
        obj = new SeqUrlGetter;
        return url_getparam_seq;
    }

    return nullptr;
}

void register_url(Functions& funcs) {

    funcs.add_poly("url_getparam", url_getparam_checker);
}

#endif
