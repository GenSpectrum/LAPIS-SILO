#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"
#include "silo/storage/lineage_index.h"

namespace silo::storage::column {
// Forward declaration for friend class access
class IndexedStringColumn;

class IndexedStringColumnPartition {
   friend class boost::serialization::access;
   friend class IndexedStringColumn;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & value_ids;
      archive & indexed_values;
      archive & lineage_index;
      // clang-format on
   }

   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   common::BidirectionalMap<std::string>& lookup;
   std::optional<LineageIndex> lineage_index;

  public:
   explicit IndexedStringColumnPartition(common::BidirectionalMap<std::string>& lookup);

   [[nodiscard]] std::optional<const roaring::Roaring*> filter(silo::Idx value_id) const;

   std::optional<const roaring::Roaring*> filter(const std::optional<std::string>& value) const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] inline std::string lookupValue(Idx id) const { return lookup.getValue(id); }

   [[nodiscard]] std::optional<silo::Idx> getValueId(const std::string& value) const;

   [[nodiscard]] const std::optional<LineageIndex>& getLineageIndex() const;
};

class IndexedStringColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & *lookup;
      // clang-format on
   }

   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;
   std::deque<IndexedStringColumnPartition> partitions;

  public:
   IndexedStringColumn();

   IndexedStringColumnPartition& createPartition();

   void generateLineageIndex(const common::LineageTreeAndIdMap& lineage_tree);
};

}  // namespace silo::storage::column
