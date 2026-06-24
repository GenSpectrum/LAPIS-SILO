#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/split_free.hpp>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/german_string.h"
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/row_id.h"
#include "silo/storage/vector/german_string_registry.h"
#include "silo/storage/vector/variable_data_registry.h"

namespace silo::storage::column {
using silo::common::TreeNodeId;

class StringColumnBuilder;

class StringColumnMetadata : public ColumnMetadata {
  public:
   silo::common::BidirectionalStringMap dictionary;

   std::optional<silo::common::PhyloTree> phylo_tree;

   explicit StringColumnMetadata(std::string column_name)
       : ColumnMetadata(std::move(column_name)) {}

   StringColumnMetadata(std::string column_name, silo::common::PhyloTree phylo_tree)
       : ColumnMetadata(std::move(column_name)),
         phylo_tree(std::move(phylo_tree)) {}

   StringColumnMetadata(std::string column_name, silo::common::BidirectionalStringMap&& dictionary)
       : ColumnMetadata(std::move(column_name)),
         dictionary(std::move(dictionary)) {}

   StringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalStringMap&& dictionary,
      silo::common::PhyloTree phylo_tree
   )
       : ColumnMetadata(std::move(column_name)),
         dictionary(std::move(dictionary)),
         phylo_tree(std::move(phylo_tree)) {}

   StringColumnMetadata() = delete;
   StringColumnMetadata(const StringColumnMetadata& other) = delete;
   StringColumnMetadata(StringColumnMetadata&& other) = delete;
   StringColumnMetadata& operator=(const StringColumnMetadata& other) = delete;
   StringColumnMetadata& operator=(StringColumnMetadata&& other) = delete;
};

/// One immutable slice of a StringColumn, produced by a single `appendChunk` call. The German
/// string suffix ids stored in `fixed_string_data` reference offsets within this same chunk's
/// `variable_string_data`, so the two registries are kept paired. A chunk is never mutated once it
/// has been appended; deleting rows rewrites whole chunks rather than shifting data across them.
class StringColumnChunk {
   vector::GermanStringRegistry fixed_string_data;

   // These pages contain the variable string suffixes. Strings that are shorter than 12 bytes are
   // stored only in `fixed_string_data`
   vector::VariableDataRegistry variable_string_data;

  public:
   /// Stores `value` and returns its row index within this chunk.
   size_t insert(std::string_view value);

   /// Stores an empty placeholder for a null value and returns its row index within this chunk.
   size_t insertNull();

   [[nodiscard]] size_t numValues() const;

   [[nodiscard]] SiloString getValue(size_t row_in_chunk) const;

   /// This includes an (re)allocation of the resulting string, one should generally
   /// work with the SiloString and @getValue instead
   [[nodiscard]] std::string lookupValue(SiloString string) const;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & fixed_string_data;
      archive & variable_string_data;
      // clang-format on
   }
};

class StringColumn {
  public:
   using Metadata = StringColumnMetadata;
   using Builder = StringColumnBuilder;
   using Buffer = std::vector<std::optional<std::string>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::STRING;
   // The type with which one can call insert
   using value_type = std::string_view;

   Metadata* metadata;

   roaring::Roaring null_bitmap;

  private:
   /// One immutable chunk per `appendChunk` call. A row id is mapped to its chunk by splitting it
   /// into a chunk id and a within-chunk index (see `RowId`): chunk `k` owns the global ids
   /// `[k << 16, (k << 16) + chunk_size)`. A `std::deque` is used because `StringColumnChunk` is
   /// move-only (its pages cannot be copied) and the deque never relocates already-appended chunks.
   std::deque<StringColumnChunk> chunks;

  public:
   explicit StringColumn(Metadata* metadata);

   std::expected<void, std::string> appendChunk(const Buffer& buffer);

   [[nodiscard]] bool isNull(RowId row_id) const;

   [[nodiscard]] SiloString getValue(RowId row_id) const;

   [[nodiscard]] std::string getValueString(RowId row_id) const;

   [[nodiscard]] size_t numChunks() const { return chunks.size(); }

   [[nodiscard]] uint32_t chunkSize(size_t chunk_idx) const {
      return chunks.at(chunk_idx).numValues();
   }

   [[nodiscard]] roaring::Roaring getDescendants(const TreeNodeId& parent) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & null_bitmap;
      archive & chunks;
      // clang-format on
   }
};

class StringColumnBuilder {
   StringColumn::Buffer buffer;

  public:
   void insert(std::string_view value);

   void insertNull();

   [[nodiscard]] size_t numValues() const;

   [[nodiscard]] StringColumn::Buffer finalize();
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::StringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& archive,
   const silo::storage::column::StringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   archive & object.column_name;
   archive & object.dictionary;
   archive & object.phylo_tree;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<silo::storage::column::StringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& archive,
   std::shared_ptr<silo::storage::column::StringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   silo::common::BidirectionalStringMap dictionary;
   std::optional<silo::common::PhyloTree> phylo_tree;
   archive & column_name;
   archive & dictionary;
   archive & phylo_tree;
   if (phylo_tree.has_value()) {
      object = std::make_shared<silo::storage::column::StringColumnMetadata>(
         std::move(column_name), std::move(dictionary), std::move(phylo_tree.value())
      );
   } else {
      object = std::make_shared<silo::storage::column::StringColumnMetadata>(
         std::move(column_name), std::move(dictionary)
      );
   }
}
}  // namespace boost::serialization
