#ifndef __TUP_FUNCS_CUTGREP_H
#define __TUP_FUNCS_CUTGREP_H

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
}

void grepif(const obj::Object* in, obj::Object*& out) {

    static std::unordered_map<std::string, std::regex> _cache;
    
    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::UInt& res = obj::get<obj::UInt>(out);

    auto i = _cache.find(regex);

    if (i == _cache.end()) {
        i = _cache.insert(i, std::make_pair(regex, std::regex(regex, std::regex_constants::optimize)));
    }

    const std::regex& r = i->second;

    bool found = std::regex_search(str, r);

    res.v = (found ? 1 : 0);
}

void register_cutgrep(Functions& funcs) {

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

    funcs.add("grepif",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::UINT),
              funcs::grepif);
}

#endif
