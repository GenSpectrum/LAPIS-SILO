#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/bidirectional_map.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/types.h"
#include "silo/storage/column/insertion_index.h"

namespace boost::serialization {
struct access;
}  // namespace boost::serialization

namespace roaring {
class Roaring;
}

namespace silo::storage::column {

template <typename SymbolType>
class InsertionColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      archive & insertion_indexes;
      // clang-format on
   }

   std::vector<silo::Idx> values;
   std::unordered_map<std::string, insertion::InsertionIndex<SymbolType>> insertion_indexes;
   common::BidirectionalMap<std::string>& lookup;

  public:
   const std::optional<std::string> default_sequence_name;

   explicit InsertionColumnPartition(
      common::BidirectionalMap<std::string>& lookup,
      const std::optional<std::string> default_sequence_name
   );

   void insert(const std::string& value);

   void insertNull();

   void buildInsertionIndexes();

   const std::unordered_map<std::string, insertion::InsertionIndex<SymbolType>>&
   getInsertionIndexes() const;

   [[nodiscard]] std::unique_ptr<roaring::Roaring> search(
      const std::string& sequence_name,
      uint32_t position,
      const std::string& search_pattern
   ) const;

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] std::string lookupValue(silo::Idx value_id) const;
};

template <typename SymbolType>
class InsertionColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & *lookup;
      archive & default_sequence_name;
      // clang-format on
   }

   std::deque<InsertionColumnPartition<SymbolType>> partitions;
   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;
   std::optional<std::string> default_sequence_name;

  public:
   InsertionColumn(std::optional<std::string> default_sequence_name);

   InsertionColumnPartition<SymbolType>& createPartition();
};

}  // namespace silo::storage::column
