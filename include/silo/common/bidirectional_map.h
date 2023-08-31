#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/pango_lineage.h"
#include "silo/common/types.h"

namespace boost::serialization {
class access;
}  // namespace boost::serialization

namespace silo::common {

template <typename V>
class BidirectionalMap {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & value_to_id;
      archive & id_to_value;
      // clang-format on
   }

   std::vector<V> id_to_value;
   std::unordered_map<V, Idx> value_to_id;

  public:
   BidirectionalMap();
   BidirectionalMap(BidirectionalMap&& map) = delete;

   [[nodiscard]] V getValue(Idx idx) const;

   [[maybe_unused]] [[nodiscard]] std::optional<Idx> getId(V value) const;

   [[nodiscard]] Idx getOrCreateId(V value);
};

}  // namespace silo::common
