#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/german_string.h"
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/storage/vector/german_string_registry.h"
#include "silo/storage/vector/variable_data_registry.h"

namespace silo::storage::column {
using silo::common::TreeNodeId;

class StringColumnMetadata : public ColumnMetadata {
  public:
   silo::common::BidirectionalStringMap dictionary;

   std::optional<silo::common::PhyloTree> phylo_tree;

   StringColumnMetadata(std::string column_name)
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

class StringColumnPartition {
  public:
   using Metadata = StringColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::STRING;
   // The type with which one can call insert
   using value_type = std::string_view;

   Metadata* metadata;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & fixed_string_data;
      archive & variable_string_data;
      // clang-format on
   }

   vector::GermanStringRegistry fixed_string_data;

   // These pages contain the variable string suffixes. Strings that are shorter than 12 bytes are
   // stored only in `fixed_string_data`
   vector::VariableDataRegistry variable_string_data;

  public:
   explicit StringColumnPartition(Metadata* metadata);

   void insert(std::string_view value);

   void insertNull();

   [[nodiscard]] bool isNull(size_t row_id) const;

   SiloString getValue(size_t row_id) const { return fixed_string_data.get(row_id); }

   std::string getValueString(size_t row_id) const {
      auto german_string = getValue(row_id);
      return lookupValue(german_string);
   }

   size_t numValues() const { return fixed_string_data.numValues(); }

   /// This includes an (re)allocation of the resulting string, one should generally
   /// work with the SiloString and @lookupSuffix instead
   [[nodiscard]] inline std::string lookupValue(SiloString string) const {
      if (string.isInPlace()) {
         auto string_view = string.getShortString();
         return std::string{string_view};
      }
      std::string result{string.prefix()};

      auto suffix_id = string.suffixId();
      vector::VariableDataRegistry::DataList suffix_chunks = variable_string_data.get(suffix_id);
      vector::VariableDataRegistry::DataList* current_chunk = &suffix_chunks;
      while (current_chunk) {
         result += current_chunk->data;
         current_chunk = current_chunk->continuation.get();
      }
      return result;
   }

   inline roaring::Roaring getDescendants(const TreeNodeId& parent) const {
      if (!metadata->phylo_tree.has_value()) {
         return roaring::Roaring();
      }
      return metadata->phylo_tree->getDescendants(parent);
   }
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::StringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::StringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
   ar & object.dictionary;
   ar & object.phylo_tree;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<silo::storage::column::StringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::StringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   silo::common::BidirectionalStringMap dictionary;
   std::optional<silo::common::PhyloTree> phylo_tree;
   ar & column_name;
   ar & dictionary;
   ar & phylo_tree;
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
