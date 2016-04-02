#ifndef __TUP_FUNCS_CUTGREP_H
#define __TUP_FUNCS_CUTGREP_H

void cut(const obj::Object* in, obj::Object*& out) {

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

void cutn(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& del = obj::get<obj::String>(args.v[1]).v;
    UInt nth = obj::get<obj::UInt>(args.v[2]).v;
    
    size_t N = str.size();
    size_t M = del.size();

    size_t prev = 0;

    std::string& v = obj::get<obj::String>(out).v;

    v.clear();

    UInt nmatch = 0;
    
    for (size_t i = 0; i < N; ++i) {

        bool matched = true;

        for (size_t j = 0; j < M; ++j) {

            if (i+j < N && str[i+j] == del[j])
                continue;
            
            matched = false;
            break;
        }

        if (matched) {

            if (nth == nmatch) {
                v.assign(str.begin() + prev, str.begin() + i);
                return;
            }

            nmatch++;
            i += M;
            prev = i;
            --i;
        }
    }

    if (nth == nmatch) {
        v.assign(str.begin() + prev, str.end());
        return;
    }

    throw std::runtime_error("Substring not found in 'cut'");
}


struct RegexCache {

    std::unordered_map<std::string, std::regex> cache;

    const std::regex& get(const std::string& s) {

        auto i = cache.find(s);

        if (i == cache.end()) {
            i = cache.insert(i, std::make_pair(s, std::regex(s, std::regex_constants::optimize)));
        }

        return i->second;
    }
};

const std::regex& regex_cache(const std::string& s) {
    static thread_local RegexCache cache;
    return cache.get(s);
}

void grep(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    const std::regex& r = regex_cache(regex);

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

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::UInt& res = obj::get<obj::UInt>(out);

    const std::regex& r = regex_cache(regex);

    bool found = std::regex_search(str, r);

    res.v = (found ? 1 : 0);
}

struct SeqGrepIf : public obj::SeqBase {

    obj::Object* seq;
    std::regex regex;

    void set_regex(const std::regex& r) {
        regex = r;
    }

    void wrap(obj::Object* s) {
        seq = s;
    }

    obj::Object* next() {

        while (1) {
            obj::Object* ret = seq->next();

            if (!ret)
                return ret;

            const std::string& str = obj::get<obj::String>(ret).v;

            if (std::regex_search(str, regex))
                return ret;
        }
    }
};

void grepif_seq(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    obj::Object* sseq = args.v[0];
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    SeqGrepIf& sgrep = obj::get<SeqGrepIf>(out);

    sgrep.set_regex(regex_cache(regex));
    sgrep.wrap(sseq);
}


void replace(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;
    const std::string& rep = obj::get<obj::String>(args.v[2]).v;
    
    std::string& res = obj::get<obj::String>(out).v;

    const std::regex& r = regex_cache(regex);

    res.clear();

    std::regex_replace(std::back_insert_iterator<std::string>(res), str.begin(), str.end(), r, rep);
}

void recut(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    const std::regex& r = regex_cache(regex);

    auto iter = str.begin();
    auto end = str.end();
    std::smatch match;
    
    while (1) {

        if (!std::regex_search(iter, end, match, r)) {
            v.emplace_back(iter, end);
            break;
        }

        v.emplace_back(iter, match[0].first);

        if (iter == match[0].second)
            throw std::runtime_error("Cannot use an empty match as a delimiter in 'recut'.");

        iter = match[0].second;

        if (iter == end) {
            v.emplace_back();
            break;
        }
    }
}

void recutn(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& regex = obj::get<obj::String>(args.v[1]).v;
    UInt nth = obj::get<obj::UInt>(args.v[2]).v;
    
    std::string& v = obj::get<obj::String>(out).v;
    v.clear();

    const std::regex& r = regex_cache(regex);

    UInt nmatch = 0;

    auto iter = str.begin();
    auto end = str.end();
    std::smatch match;
    
    while (iter != end) {

        if (!std::regex_search(iter, end, match, r)) {
            break;
        }

        if (iter == match[0].second)
            throw std::runtime_error("Cannot use an empty match as a delimiter in 'recut'.");

        if (nmatch == nth) {
            v.assign(iter, match[0].first);
            return;
        }
        
        iter = match[0].second;
        ++nmatch;
    }

    if (nmatch == nth) {
        v.assign(iter, end);
        return;
    }

    throw std::runtime_error("Substring not found in 'recut'");
}

Functions::func_t grepif_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& t1 = args.tuple->at(0);
    const Type& t2 = args.tuple->at(1);

    if (!check_string(t2))
        return nullptr;

    if (check_string(t1)) {

        ret = Type(Type::UINT);
        return grepif;
    }

    if (t1.type == Type::SEQ && t1.tuple && t1.tuple->size() == 1) {

        const Type& t3 = t1.tuple->at(0);

        if (!check_string(t3))
            return nullptr;

        ret = Type(Type::SEQ);
        ret.push(Type(Type::STRING));
        obj = new SeqGrepIf;
        return grepif_seq;
    }

    return nullptr;
}


void register_cutgrep(Functions& funcs) {

    funcs.add("cut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              cut);
    
    funcs.add("cut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::UINT) }),
              Type(Type::STRING),
              cutn);

    funcs.add("grep",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              grep);

    funcs.add_poly("grepif", grepif_checker);

    funcs.add("replace",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::STRING),
              replace);

    funcs.add("recut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              recut);

    funcs.add("recut",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::UINT) }),
              Type(Type::STRING),
              recutn);

}

#endif
