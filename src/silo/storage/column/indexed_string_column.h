#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/lineage_index.h"

namespace silo::storage::column {

class IndexedStringColumnMetadata : public ColumnMetadata {
  public:
   common::BidirectionalMap<std::string> dictionary;
   std::optional<common::LineageTreeAndIdMap> lineage_tree;

   IndexedStringColumnMetadata(std::string column_name)
       : ColumnMetadata(column_name) {}

   IndexedStringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalMap<std::string> dictionary
   )
       : ColumnMetadata(column_name),
         dictionary(std::move(dictionary)) {}

   IndexedStringColumnMetadata(
      std::string column_name,
      common::LineageTreeAndIdMap lineage_tree_and_id_map
   );

   IndexedStringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalMap<std::string> dictionary,
      common::LineageTreeAndIdMap lineage_tree_and_id_map
   );

   IndexedStringColumnMetadata() = delete;
   IndexedStringColumnMetadata(const IndexedStringColumnMetadata& other) = delete;
   IndexedStringColumnMetadata(IndexedStringColumnMetadata&& other) = delete;
   IndexedStringColumnMetadata& operator=(const IndexedStringColumnMetadata& other) = delete;
   IndexedStringColumnMetadata& operator=(IndexedStringColumnMetadata&& other) = delete;
};

class IndexedStringColumnPartition {
  public:
   using Metadata = IndexedStringColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::INDEXED_STRING;

  private:
   friend class boost::serialization::access;
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
   Metadata* metadata;

  public:
   explicit IndexedStringColumnPartition(Metadata* metadata);

  public:
   [[nodiscard]] std::optional<const roaring::Roaring*> filter(silo::Idx value_id) const;

   std::optional<const roaring::Roaring*> filter(const std::optional<std::string>& value) const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] inline std::string lookupValue(Idx id) const {
      return metadata->dictionary.getValue(id);
   }

   [[nodiscard]] std::optional<silo::Idx> getValueId(const std::string& value) const;

   [[nodiscard]] const std::optional<LineageIndex>& getLineageIndex() const;
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::IndexedStringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::IndexedStringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
   ar & object.dictionary;
   ar & object.lineage_tree;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<silo::storage::column::IndexedStringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::IndexedStringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   silo::common::BidirectionalMap<std::string> dictionary;
   std::optional<silo::common::LineageTreeAndIdMap> lineage_tree;
   ar & column_name;
   ar & dictionary;
   ar & lineage_tree;
   if (lineage_tree.has_value()) {
      object = std::make_shared<silo::storage::column::IndexedStringColumnMetadata>(
         std::move(column_name), std::move(dictionary), std::move(lineage_tree.value())
      );
   } else {
      object = std::make_shared<silo::storage::column::IndexedStringColumnMetadata>(
         std::move(column_name), std::move(dictionary)
      );
   }
}
}  // namespace boost::serialization
