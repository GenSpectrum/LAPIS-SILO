#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/bidirectional_map.h"
#include "silo/common/string.h"

namespace silo::storage::column {

class StringColumnPartition {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      archive & values;
      // clang-format on
   }

   std::string column_name;
   std::vector<common::String<silo::common::STRING_SIZE>> values;
   silo::common::BidirectionalMap<std::string>* lookup;

  public:
   explicit StringColumnPartition(
      std::string column_name,
      silo::common::BidirectionalMap<std::string>* lookup
   );

   [[nodiscard]] const std::vector<common::String<silo::common::STRING_SIZE>>& getValues() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] std::optional<common::String<silo::common::STRING_SIZE>> embedString(
      const std::string& string
   ) const;

   [[nodiscard]] inline std::string lookupValue(common::String<silo::common::STRING_SIZE> string
   ) const {
      return string.toString(*lookup);
   }
};

class StringColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      archive & lookup;
      // clang-format on
   }

   std::string column_name;
   silo::common::BidirectionalMap<std::string> lookup;
   // Need container with pointer stability, because database partitions point into this
   std::deque<StringColumnPartition> partitions;

  public:
   explicit StringColumn(std::string column_name);

   StringColumn(const StringColumn& other) = delete;
   StringColumn(StringColumn&& other) = delete;
   StringColumn& operator=(const StringColumn& other) = delete;
   StringColumn& operator=(StringColumn&& other) = delete;

   StringColumnPartition& createPartition();

   [[nodiscard]] std::optional<common::String<silo::common::STRING_SIZE>> embedString(
      const std::string& string
   ) const;
};

}  // namespace silo::storage::column
