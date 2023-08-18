#ifndef SILO_NUCLEOTIDE_SYMBOL_MAP_H
#define SILO_NUCLEOTIDE_SYMBOL_MAP_H

#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/nucleotide_symbols.h"

namespace silo {

template <typename T>
class NucleotideSymbolMap {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & data;
      // clang-format on
   }

   std::array<T, Util<NUCLEOTIDE_SYMBOL>::count> data;

  public:
   inline T& operator[](NUCLEOTIDE_SYMBOL symbol) { return data.at(static_cast<uint8_t>(symbol)); }

   inline const T& at(NUCLEOTIDE_SYMBOL symbol) const {
      return data.at(static_cast<uint8_t>(symbol));
   }
};

}  // namespace silo

#endif  // SILO_NUCLEOTIDE_SYMBOL_MAP_H
