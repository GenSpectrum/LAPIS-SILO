#ifndef SILO_HASHING_H
#define SILO_HASHING_H

#include <cstdint>

namespace silo::common {

inline uint64_t murmurHash(uint64_t k) {
   // MurmurHash64A
   const uint64_t m = 0xc6a4a7935bd1e995;
   const int r = 47;
   uint64_t h = 0x8445d61a4e774912 ^ (8 * m);
   k *= m;
   k ^= k >> r;
   k *= m;
   h ^= k;
   h *= m;
   h ^= h >> r;
   h *= m;
   h ^= h >> r;
   return h;
}

}  // namespace silo::common

#endif  // SILO_HASHING_H
