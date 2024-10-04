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
      if(lineage_index.has_value()){
         archive & lineage_index.value();
      }
      // clang-format on
   }

   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   std::optional<LineageIndex> lineage_index;
   common::BidirectionalMap<std::string>* lookup;

  public:
   explicit IndexedStringColumnPartition(common::BidirectionalMap<std::string>* lookup);

   explicit IndexedStringColumnPartition(
      common::BidirectionalMap<std::string>* lookup,
      const common::LineageTree* lineage_tree
   );

   [[nodiscard]] std::optional<const roaring::Roaring*> filter(silo::Idx value_id) const;

   std::optional<const roaring::Roaring*> filter(const std::optional<std::string>& value) const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] inline std::string lookupValue(Idx id) const { return lookup->getValue(id); }

   [[nodiscard]] std::optional<silo::Idx> getValueId(const std::string& value) const;

   [[nodiscard]] const std::optional<LineageIndex>& getLineageIndex() const;
};

class IndexedStringColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & lookup;
      archive & lineage_tree;
      // clang-format on
   }

   common::BidirectionalMap<std::string> lookup;
   std::optional<common::LineageTree> lineage_tree;
   std::deque<IndexedStringColumnPartition> partitions;

  public:
   IndexedStringColumn();

   IndexedStringColumn(const common::LineageTreeAndIdMap& lineage_tree_and_id_map);

   IndexedStringColumnPartition& createPartition();
};

}  // namespace silo::storage::column
