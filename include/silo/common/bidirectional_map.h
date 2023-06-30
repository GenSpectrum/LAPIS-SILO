#ifndef SILO_BIDIRECTIONAL_MAP_H
#define SILO_BIDIRECTIONAL_MAP_H

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/types.h"

namespace silo::common {

template <typename V>
class BidirectionalMap {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& value_to_id;
      archive& id_to_value;
      // clang-format on
   }

   std::vector<V> id_to_value;
   std::unordered_map<V, Idx> value_to_id;

  public:
   BidirectionalMap();
   BidirectionalMap(BidirectionalMap&& map) = delete;

   [[nodiscard]] V getValue(Idx idx) const;

   [[nodiscard]] std::optional<Idx> getId(V value) const;

   [[nodiscard]] Idx getOrCreateId(V value);
};

}  // namespace silo::common

#endif  // SILO_BIDIRECTIONAL_MAP_H
