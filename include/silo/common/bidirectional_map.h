#pragma once

#include <cstdint>
#include <functional>
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
   BidirectionalMap()
       : id_to_value(),
         value_to_id() {}

   BidirectionalMap(std::vector<V>&& id_to_value, std::unordered_map<V, Idx>&& value_to_id)
       : id_to_value(id_to_value),
         value_to_id(value_to_id) {}

   BidirectionalMap(BidirectionalMap&& map) = default;
   BidirectionalMap& operator=(BidirectionalMap&& map) = default;

   // Copy constructor and operator are deleted to restrict mistaken copies of this data-structure
   // When this data-structure is copied, the two copies' dictionary will diverge over-time
   // If you want to copy this data-structure, explicitly call BidirectionalMap::copy() and
   // make sure that the contained type's copy is permissible for your design.
   BidirectionalMap(const BidirectionalMap& map) = delete;
   BidirectionalMap& operator=(const BidirectionalMap& map) = delete;

   BidirectionalMap copy() const {
      BidirectionalMap result;
      result.id_to_value = id_to_value;
      result.value_to_id = value_to_id;
      return result;
   }

   V getValue(Idx idx) const { return id_to_value.at(idx); }

   std::optional<Idx> getId(const V& value) const {
      if (value_to_id.contains(value)) {
         return value_to_id.at(value);
      }
      return std::nullopt;
   }

   Idx getOrCreateId(V value) {
      if (value_to_id.contains(value)) {
         return value_to_id.at(value);
      }
      const Idx identifier = id_to_value.size();
      id_to_value.push_back(value);
      value_to_id[value] = identifier;
      return identifier;
   }
};

}  // namespace silo::common
