#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <yaml-cpp/yaml.h>
#include <boost/serialization/split_member.hpp>

#include "silo/common/types.h"

namespace silo::common {

class BidirectionalStringMap {
   // This is an implicit map from the integers [0, id_to_value.size())
   // to the strings at the respective index in the vector
   std::vector<std::string> id_to_value;
   // This is an explicit map from string_views to their index in the vector
   // The views point to strings that are stored in the vector
   std::unordered_map<std::string_view, Idx> value_to_id;

   void fillLookupFromVector() {
      value_to_id.clear();
      value_to_id.reserve(id_to_value.size());
      for (size_t id = 0; id < id_to_value.size(); ++id) {
         const std::string_view value_for_id = id_to_value.at(id);
         value_to_id[value_for_id] = id;
      }
   };

  public:
   BidirectionalStringMap() = default;

   // We can use the default because of https://en.cppreference.com/w/cpp/container/vector/vector:
   // > After container move construction, references, pointers, and iterators
   // > (other than the end iterator) to other remain valid, but refer to elements
   // > that are now in *this.
   BidirectionalStringMap(BidirectionalStringMap&& other) = default;
   BidirectionalStringMap& operator=(BidirectionalStringMap&& other) = default;

   // Copy constructor and operator are deleted to restrict mistaken copies of this data-structure
   // When this data-structure is copied, the two copies' dictionary will diverge over-time
   // If you want to copy this data-structure, explicitly call BidirectionalStringMap::copy() and
   // make sure that the contained type's copy is permissible for your design.
   BidirectionalStringMap(const BidirectionalStringMap& other) = delete;
   BidirectionalStringMap& operator=(const BidirectionalStringMap& other) = delete;

   [[nodiscard]] BidirectionalStringMap copy() const {
      BidirectionalStringMap result;
      result.id_to_value = id_to_value;
      result.fillLookupFromVector();
      return result;
   }

   [[nodiscard]] std::string_view getValue(Idx idx) const { return id_to_value.at(idx); }

   [[nodiscard]] std::optional<Idx> getId(std::string_view value) const {
      if (value_to_id.contains(value)) {
         return value_to_id.at(value);
      }
      return std::nullopt;
   }

   Idx getOrCreateId(std::string_view value) {
      if (value_to_id.contains(value)) {
         return value_to_id.at(value);
      }
      const Idx identifier = id_to_value.size();
      const bool will_reallocate = id_to_value.capacity() == id_to_value.size();
      id_to_value.emplace_back(value);
      if (will_reallocate) {
         fillLookupFromVector();
      } else {
         value_to_id[id_to_value.back()] = identifier;
      }
      return identifier;
   }

   template <class Archive>
   void save(Archive& archive, const uint32_t /* version */) const {
      archive & id_to_value;
   }

   template <class Archive>
   void load(Archive& archive, const uint32_t /* version */) {
      archive & id_to_value;

      fillLookupFromVector();
   }

   BOOST_SERIALIZATION_SPLIT_MEMBER()
};

}  // namespace silo::common
