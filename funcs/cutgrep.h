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

template <void CUTTER(const obj::Object*, obj::Object*&)>
struct SeqCut : public obj::SeqBase {

    obj::Object* seq;
    obj::Object* ret;
    obj::Tuple* inp;

    SeqCut() : ret(new obj::ArrayAtom<std::string>) {}

    void do_wrap(obj::Tuple* i) {
        inp = i;
        seq = inp->v[0];
    }

    obj::Object* next() {

        while (1) {
            obj::Object* n = seq->next();

            if (!n)
                return n;

            inp->v[0] = n;
            CUTTER(inp, ret);
            return ret;
        }
    }
};

void cut_seq(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    SeqCut<cut>& ret = obj::get< SeqCut<cut> >(out);

    ret.do_wrap(&args);
}

Functions::func_t cut_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args == Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) })) {
        ret = Type(Type::ARR, { Type::STRING });
        return cut;
    }

    if (args == Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::UINT) })) {
        ret = Type(Type::STRING);
        return cutn;
    }

    if (args == Type(Type::TUP, { Type(Type::SEQ, { Type(Type::STRING) }), Type(Type::STRING) })) {
        ret = Type(Type::SEQ, { Type(Type::ARR, { Type::STRING }) });

        obj = new SeqCut<cut>;
        return cut_seq;
    }

    return nullptr;
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

template <bool REGEX>
struct Searcher;

template <>
struct Searcher<true> {

    std::reference_wrapper<const std::regex> rx;

    Searcher(const std::string& p) : rx(std::cref(regex_cache(p))) {}

    bool matches(const std::string& s) {
        return std::regex_search(s, rx.get());
    }

    void matches(const std::string& s, std::vector<std::string>& v) {

        std::sregex_iterator iter(s.begin(), s.end(), rx);
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
};

template <>
struct Searcher<false> {

    std::reference_wrapper<const std::string> substr;

    Searcher(const std::string& p) : substr(std::cref(p)) {}

    bool matches(const std::string& s) {
        return (s.find(substr) != std::string::npos);
    }

    void matches(const std::string& s, std::vector<std::string>& v) {

        if (matches(s)) {
            v.emplace_back(substr);
        }
    }
};

template <bool REGEX>
void grep(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& pattern = obj::get<obj::String>(args.v[1]).v;

    obj::ArrayAtom<std::string>& vv = obj::get< obj::ArrayAtom<std::string> >(out);
    std::vector<std::string>& v = vv.v;

    v.clear();

    Searcher<REGEX> searcher(pattern);

    searcher.matches(str, v);
}

template <bool REGEX>
void grepif(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);
    
    const std::string& str = obj::get<obj::String>(args.v[0]).v;
    const std::string& pattern = obj::get<obj::String>(args.v[1]).v;

    obj::UInt& res = obj::get<obj::UInt>(out);

    Searcher<REGEX> searcher(pattern);

    bool found = searcher.matches(str);

    res.v = (found ? 1 : 0);
}

template <bool REGEX>
struct SeqGrepIf : public obj::SeqBase {

    obj::Object* seq;
    Searcher<REGEX> searcher;

    SeqGrepIf() : searcher("") {}

    void set_pattern(const std::string& p) {
        searcher = Searcher<REGEX>(p);
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

            if (searcher.matches(str))
                return ret;
        }
    }
};

template <bool REGEX>
void grepif_seq(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    obj::Object* sseq = args.v[0];
    const std::string& patt = obj::get<obj::String>(args.v[1]).v;

    SeqGrepIf<REGEX>& sgrep = obj::get< SeqGrepIf<REGEX> >(out);

    sgrep.set_pattern(patt);
    sgrep.wrap(sseq);
}

template <bool REGEX>
Functions::func_t grepif_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type != Type::TUP || !args.tuple || args.tuple->size() != 2)
        return nullptr;

    const Type& t1 = args.tuple->at(0);
    const Type& t2 = args.tuple->at(1);

    if (!check_string(t2))
        return nullptr;

    if (check_string(t1)) {

        ret = Type(Type::UINT);
        return grepif<REGEX>;
    }

    if (t1.type == Type::SEQ && t1.tuple && t1.tuple->size() == 1) {

        const Type& t3 = t1.tuple->at(0);

        if (!check_string(t3))
            return nullptr;

        ret = Type(Type::SEQ);
        ret.push(Type(Type::STRING));
        obj = new SeqGrepIf<REGEX>;
        return grepif_seq<REGEX>;
    }

    return nullptr;
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

void recut_seq(const obj::Object* in, obj::Object*& out) {

    obj::Tuple& args = obj::get<obj::Tuple>(in);

    SeqCut<recut>& ret = obj::get< SeqCut<recut> >(out);

    ret.wrap(&args);
}

Functions::func_t recut_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args == Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) })) {
        ret = Type(Type::ARR, { Type::STRING });
        return recut;
    }

    if (args == Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::UINT) })) {
        ret = Type(Type::STRING);
        return recutn;
    }

    if (args == Type(Type::TUP, { Type(Type::SEQ, { Type(Type::STRING) }), Type(Type::STRING) })) {
        ret = Type(Type::SEQ, { Type(Type::ARR, { Type::STRING }) });

        obj = new SeqCut<recut>;
        return recut_seq;
    }
   
    return nullptr;
}

void register_cutgrep(Functions& funcs) {

    funcs.add_poly("cut", cut_checker);

    funcs.add("grep",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              grep<true>);

    funcs.add_poly("grepif", grepif_checker<true>);

    funcs.add("find",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::ARR, { Type::STRING }),
              grep<false>);

    funcs.add_poly("findif", grepif_checker<false>);

    funcs.add("replace",
              Type(Type::TUP, { Type(Type::STRING), Type(Type::STRING), Type(Type::STRING) }),
              Type(Type::STRING),
              replace);

    funcs.add_poly("recut", recut_checker);

}

#endif
