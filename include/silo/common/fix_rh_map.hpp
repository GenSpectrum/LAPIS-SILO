//
// Created by Alexander Taepper on 25.11.22.
//

#ifndef SILO_FIXHASHTABLE_H
#define SILO_FIXHASHTABLE_H

#include "hashing.h"
#include <climits>
#include <vector>

namespace silo {

template <typename KEY, typename VALUE>
struct fix_rh_map {
   struct Entry {
      KEY key;
      VALUE value;
      unsigned int psl;
      explicit Entry(KEY key, VALUE value, uint64_t psl) : key(std::move(key)), value(std::move(value)), psl(psl) {}
   };
   uint64_t htSize;
   uint64_t mask;
   std::vector<Entry> ht;
   size_t count = 0;

   explicit fix_rh_map(uint64_t size) {
      htSize = 1ull << ((8 * sizeof(uint64_t)) - __builtin_clzl(size)); // next power of two
      mask = htSize - 1;
      ht.resize(htSize, Entry(KEY(), VALUE(), UINT_MAX));
   }

   Entry* lookup(KEY key, uint64_t hash) {
      uint64_t c = hash & mask;
      uint64_t psl = 0;
      while (true) {
         if (ht[c].psl == UINT_MAX) return nullptr;
         if (ht[c].key == key) return &ht[c];
         if (psl > ht[c].psl) return nullptr;
         c = (c + 1) & mask;
         psl++;
      }
   }

   const Entry* lookup_f(const KEY& key, uint64_t hash) const {
      uint64_t c = hash & mask;
      while (true) {
         if (ht[c].psl == UINT_MAX) return nullptr;
         if (ht[c].key == key) return &ht[c];
         c = (c + 1) & mask;
      }
   }

   bool isFull() {
      return count == htSize;
   }

   bool insert(KEY key, VALUE val, uint64_t hash) {
      uint64_t c = hash & mask;
      uint64_t psl = 0;
      while (psl < htSize) {
         if (ht[c].psl == UINT_MAX) {
            ht[c] = Entry(key, val, psl);
            count++;
            return false;
         }
         if (ht[c].key == key) {
            return true;
         }
         if (psl > ht[c].psl) {
            auto tmp = ht[c];
            ht[c] = Entry(key, val, psl);
            key = tmp.key;
            val = tmp.value;
            psl = tmp.psl;
         }
         c = (c + 1) & mask;
         psl++;
      }
      return false;
   }
};
}

#include <cassert>

inline void testRHHt() {
   using namespace silo;
   for (uint64_t size : {10, 99, 837, 48329, 384933}) {
      fix_rh_map<uint64_t, uint64_t> h(size);
      // insert 1
      for (uint64_t i = 0; i < size / 2; i++) {
         assert(!h.insert(i, i + 1, murmurHash64(i)));
      }
      // lookup
      for (uint64_t i = 0; i < size; i++) {
         fix_rh_map<uint64_t, uint64_t>::Entry* e = h.lookup(i, murmurHash64(i));
         if (i < size / 2) {
            assert(e);
            assert(e->value == i + 1);
         } else {
            assert(!e);
         }
      }
      // insert 2
      for (uint64_t i = size / 2; i < size; i++) {
         assert(!h.insert(i, i + 1, murmurHash64(i)));
      }
      // lookup
      for (uint64_t i = 0; i < size; i++) {
         fix_rh_map<uint64_t, uint64_t>::Entry* e = h.lookup(i, murmurHash64(i));
         assert(e);
         assert(e->value == i + 1);
      }
   }
}
#endif //SILO_FIXHASHTABLE_H
