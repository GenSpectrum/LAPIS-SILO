#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/phylo_tree.h"
#include "silo/common/string.h"
#include "silo/common/tree_node_id.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {
using silo::common::MRCAResponse;
using silo::common::TreeNodeId;

class StringColumnMetadata : public ColumnMetadata {
  public:
   silo::common::BidirectionalStringMap dictionary;

   std::optional<silo::common::PhyloTree> phylo_tree;

   StringColumnMetadata(std::string phylo_tree_field)
       : ColumnMetadata(std::move(phylo_tree_field)) {}

   StringColumnMetadata(std::string phylo_tree_field, silo::common::PhyloTree phylo_tree)
       : ColumnMetadata(std::move(phylo_tree_field)),
         phylo_tree(std::move(phylo_tree)) {}

   StringColumnMetadata(std::string phylo_tree_field, silo::common::BidirectionalStringMap&& dictionary)
       : ColumnMetadata(std::move(phylo_tree_field)),
         dictionary(std::move(dictionary)) {}

   StringColumnMetadata(
      std::string phylo_tree_field,
      silo::common::BidirectionalStringMap&& dictionary,
      silo::common::PhyloTree phylo_tree
   )
       : ColumnMetadata(std::move(phylo_tree_field)),
         dictionary(std::move(dictionary)),
         phylo_tree(std::move(phylo_tree)) {}

   StringColumnMetadata() = delete;
   StringColumnMetadata(const StringColumnMetadata& other) = delete;
   StringColumnMetadata(StringColumnMetadata&& other) = delete;
   StringColumnMetadata& operator=(const StringColumnMetadata& other) = delete;
   StringColumnMetadata& operator=(StringColumnMetadata&& other) = delete;

   [[nodiscard]] std::optional<common::String<silo::common::STRING_SIZE>> embedString(
      const std::string& string
   ) const;

   inline MRCAResponse getMRCA(const std::vector<std::string>& node_labels) const {
      if (!phylo_tree.has_value()) {
         return MRCAResponse{
            std::nullopt,
            {},
         };
      }
      return phylo_tree->getMRCA(node_labels);
   }
};

class StringColumnPartition {
  public:
   using Metadata = StringColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::STRING;

   Metadata* metadata;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<common::String<silo::common::STRING_SIZE>> values;

  public:
   explicit StringColumnPartition(Metadata* metadata);

   [[nodiscard]] const std::vector<common::String<silo::common::STRING_SIZE>>& getValues() const;

   void insert(std::string_view value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] std::optional<common::String<silo::common::STRING_SIZE>> embedString(
      const std::string& string
   ) const;

   [[nodiscard]] inline std::string lookupValue(common::String<silo::common::STRING_SIZE> string
   ) const {
      return string.toString(metadata->dictionary);
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
   ar & object.phylo_tree_field;
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
   std::string phylo_tree_field;
   silo::common::BidirectionalStringMap dictionary;
   std::optional<silo::common::PhyloTree> phylo_tree;
   ar & phylo_tree_field;
   ar & dictionary;
   ar & phylo_tree;
   if (phylo_tree.has_value()) {
      object = std::make_shared<silo::storage::column::StringColumnMetadata>(
         std::move(phylo_tree_field), std::move(dictionary), std::move(phylo_tree.value())
      );
   } else {
      object = std::make_shared<silo::storage::column::StringColumnMetadata>(
         std::move(phylo_tree_field), std::move(dictionary)
      );
   }
}
}  // namespace boost::serialization
