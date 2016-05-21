#ifndef __TUP_FUNCS_UNIQUE_H
#define __TUP_FUNCS_UNIQUE_H

// Count unique elements using a hash table.
// (Almost) Exact, but uses memory linearly.

struct AtomUniques : public obj::UInt {

    std::unordered_set<UInt> count;

    obj::Object* clone() const {
        AtomUniques* ret = new AtomUniques;
        ret->v = v;
        ret->count = count;
        return ret;
    }

    void insert(const obj::Object* o) {
        v = 1;
        count.clear();
        count.insert(o->hash());
    }

    void merge(const obj::Object* o) {
        AtomUniques& x = obj::get<AtomUniques>(o);
        count.insert(x.count.begin(), x.count.end());
    }

    void merge_end() {
        v = count.size();
    }
};


// Count unique elements using the HyperLogLog statistical estimator.
// Introduces an error but uses a constant amount of memory.
// Not very good for counting small sets of values!

template <size_t NumBucketBits = 12>
struct AtomUniquesEstimate : public obj::UInt {

    std::unordered_map<UInt,uint8_t> bits;

    AtomUniquesEstimate() : obj::UInt(0), bits() {}

    obj::Object* clone() const {
        AtomUniquesEstimate<NumBucketBits>* ret = new AtomUniquesEstimate<NumBucketBits>;
        ret->v = v;
        ret->bits = bits;
        return ret;
    }

    uint8_t count_trailing_zeros(UInt n) {
        uint8_t ret = 1;
        size_t mask = 1;

        while (ret < 255) {
            if (mask & n)
                break;

            ret++;
            mask <<= 1;
        }

        return ret;
    }

    void insert(const obj::Object* o) {
        v = 1;
        bits.clear();

        const UInt h = o->hash();
        const UInt h0 = h & ((1 << NumBucketBits) - 1);
        const UInt h1 = h >> NumBucketBits;

        bits[h0] = count_trailing_zeros(h1);
    }

    void merge(const obj::Object* o) {
        AtomUniquesEstimate<NumBucketBits>& x = obj::get< AtomUniquesEstimate<NumBucketBits> >(o);

        for (const auto& i : x.bits) {
            bits[i.first] = std::max(bits[i.first], i.second);
        }
    }

    void merge_end() {

        Real x = 0;
        for (const auto& i : bits) {
            x += i.second;
        }

        v = ::round(0.39701 * bits.size() * pow(2, x / bits.size()));
    }
};


template <typename T>
void uniques(const obj::Object* in, obj::Object*& out) {
    obj::get<T>(out).insert(in);
}

template <typename T>
Functions::func_t uniques_checker(const Type& args, Type& ret, obj::Object*& obj) {

    if (args.type == Type::SEQ || args.type == Type::NONE)
        return nullptr;

    ret = Type(Type::UINT);
    obj = new T;

    return uniques<T>;
}

void register_uniques(Functions& funcs) {

    funcs.add_poly("uniques", uniques_checker<AtomUniques>);
    funcs.add_poly("uniques_estimate", uniques_checker< AtomUniquesEstimate<> >);
}


#endif

