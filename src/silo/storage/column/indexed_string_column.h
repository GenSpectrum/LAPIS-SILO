#pragma once

#include <cstdint>
#include <deque>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/lineage_index.h"

namespace silo::storage::column {

class IndexedStringColumnBuilder;

class IndexedStringColumnMetadata : public ColumnMetadata {
  public:
   common::BidirectionalStringMap dictionary;
   std::optional<common::LineageTreeAndIdMap> lineage_tree;
   bool treat_unknown_lineages_as_null = false;

   explicit IndexedStringColumnMetadata(std::string column_name)
       : ColumnMetadata(std::move(column_name)) {}

   IndexedStringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalStringMap dictionary
   )
       : ColumnMetadata(std::move(column_name)),
         dictionary(std::move(dictionary)) {}

   IndexedStringColumnMetadata(
      std::string column_name,
      common::LineageTreeAndIdMap lineage_tree_and_id_map,
      bool treat_unknown_lineages_as_null
   );

   IndexedStringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalStringMap dictionary,
      common::LineageTreeAndIdMap lineage_tree_and_id_map,
      bool treat_unknown_lineages_as_null
   );

   IndexedStringColumnMetadata() = delete;
   IndexedStringColumnMetadata(const IndexedStringColumnMetadata& other) = delete;
   IndexedStringColumnMetadata(IndexedStringColumnMetadata&& other) = delete;
   IndexedStringColumnMetadata& operator=(const IndexedStringColumnMetadata& other) = delete;
   IndexedStringColumnMetadata& operator=(IndexedStringColumnMetadata&& other) = delete;
};

class IndexedStringColumn {
  public:
   using Metadata = IndexedStringColumnMetadata;
   using Builder = IndexedStringColumnBuilder;
   using Buffer = std::vector<std::optional<std::string>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::INDEXED_STRING;
   using value_type = std::string_view;

   Metadata* metadata;
   roaring::Roaring null_bitmap;

  private:
   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   std::optional<LineageIndex> lineage_index;

  public:
   explicit IndexedStringColumn(Metadata* metadata);

   [[nodiscard]] std::optional<const roaring::Roaring*> filter(silo::Idx value_id) const;

   [[nodiscard]] std::optional<const roaring::Roaring*> filter(
      const std::optional<std::string>& value
   ) const;

   std::expected<void, std::string> appendChunk(const Buffer& buffer);

   [[nodiscard]] size_t numValues() const { return value_ids.size(); }

   [[nodiscard]] const silo::Idx& getValue(size_t row_id) const { return value_ids.at(row_id); }

   [[nodiscard]] bool isNull(size_t row_id) const;

   [[nodiscard]] std::string getValueString(size_t row_id) const {
      return std::string{lookupValue(getValue(row_id))};
   }

   [[nodiscard]] std::string_view lookupValue(Idx dict_id) const {
      return metadata->dictionary.getValue(dict_id);
   }

   [[nodiscard]] std::optional<silo::Idx> getValueId(const std::string& value) const;

   [[nodiscard]] const std::optional<LineageIndex>& getLineageIndex() const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & value_ids;
      archive & indexed_values;
      archive & null_bitmap;
      if(lineage_index.has_value()){
         archive & lineage_index.value();
      }
      // clang-format on
   }
};

class IndexedStringColumnBuilder {
   IndexedStringColumn::Buffer buffer;

  public:
   void insert(std::string_view value) { buffer.emplace_back(std::string{value}); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] IndexedStringColumn::Buffer finalize() {
      IndexedStringColumn::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::IndexedStringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& archive,
   const silo::storage::column::IndexedStringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   archive & object.column_name;
   archive & object.dictionary;
   archive & object.lineage_tree;
   archive & object.treat_unknown_lineages_as_null;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<silo::storage::column::IndexedStringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& archive,
   std::shared_ptr<silo::storage::column::IndexedStringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   silo::common::BidirectionalStringMap dictionary;
   std::optional<silo::common::LineageTreeAndIdMap> lineage_tree;
   bool treat_unknown_lineages_as_null;
   archive & column_name;
   archive & dictionary;
   archive & lineage_tree;
   archive & treat_unknown_lineages_as_null;
   if (lineage_tree.has_value()) {
      object = std::make_shared<silo::storage::column::IndexedStringColumnMetadata>(
         std::move(column_name),
         std::move(dictionary),
         std::move(lineage_tree.value()),
         treat_unknown_lineages_as_null
      );
   } else {
      object = std::make_shared<silo::storage::column::IndexedStringColumnMetadata>(
         std::move(column_name), std::move(dictionary)
      );
   }
}
}  // namespace boost::serialization
