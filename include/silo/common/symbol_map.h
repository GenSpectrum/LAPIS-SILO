#ifndef SILO_SYMBOL_MAP_H
#define SILO_SYMBOL_MAP_H

#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo {

template <typename Symbol, typename T>
class SymbolMap {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & data;
      // clang-format on
   }

   std::array<T, Util<Symbol>::count> data;

  public:
   T& operator[](Symbol symbol) { return data.at(static_cast<uint8_t>(symbol)); }

   const T& at(Symbol symbol) const { return data.at(static_cast<uint8_t>(symbol)); }
};

}  // namespace silo

#endif  // SILO_SYMBOL_MAP_H
