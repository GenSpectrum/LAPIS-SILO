//
// Created by Alexander Taepper on 25.11.22.
//

#ifndef SILO_HASHING_H
#define SILO_HASHING_H

#include <stdint.h>
#include <string>

inline constexpr uint64_t murmurHash64(uint64_t k) {
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

inline uint64_t hash_string(const std::string& x) {
   unsigned result = 0;
   for (char c : x) {
      result = ((result << 5) | (result >> 27)) ^ (static_cast<unsigned char>(c));
   }
   return murmurHash64(result);
}

#endif  // SILO_HASHING_H
