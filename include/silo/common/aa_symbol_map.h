#ifndef SILO_AA_SYMBOL_MAP_H
#define SILO_AA_SYMBOL_MAP_H

#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/aa_symbols.h"

namespace silo {

template <typename T>
class AASymbolMap {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive& data;
      // clang-format on
   }

   std::array<T, AA_SYMBOL_COUNT> data;

  public:
   inline T& operator[](AA_SYMBOL symbol) { return data.at(static_cast<uint8_t>(symbol)); }
   inline const T& at(AA_SYMBOL symbol) const { return data.at(static_cast<uint8_t>(symbol)); }
};

}  // namespace silo

#endif  // SILO_AA_SYMBOL_MAP_H
