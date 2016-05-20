#ifndef __TAB_HASH_H
#define __TAB_HASH_H

namespace tab {

typedef UInt hash_t;

hash_t do_hash(const unsigned char* c, size_t n, hash_t init, hash_t mul) {

    hash_t hash = init;

    while (n > 0) {
        hash ^= (hash_t)(*c);
        hash *= (hash_t)mul;
        ++c;
        --n;
    }

    return hash;
}

constexpr hash_t fnv_basis() {
    return (sizeof(hash_t) >= 8 ? 0xcbf29ce484222325 : 0x811c9dc5);
}

constexpr hash_t fnv_prime() {
    return (sizeof(hash_t) >= 8 ? 0x100000001b3 : 0x01000193);
}

template <typename T>
hash_t do_hash(const T& v, hash_t basis) {
    return do_hash(reinterpret_cast<const unsigned char*>(&v), sizeof(v), basis, fnv_prime());
}

template <>
hash_t do_hash<std::string>(const std::string& v, hash_t basis) {
    return do_hash(reinterpret_cast<const unsigned char*>(v.data()), v.size(), basis, fnv_prime());
}

}

#endif
