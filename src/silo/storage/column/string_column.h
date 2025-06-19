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
#include "silo/preprocessing/phylo_tree_file.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class StringColumnMetadata : public ColumnMetadata {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & dictionary;
      // clang-format on
   }

  public:
   silo::common::BidirectionalMap<std::string> dictionary;

   std::optional<silo::preprocessing::PhyloTreeFile> phylo_tree;

   StringColumnMetadata(std::string column_name)
       : ColumnMetadata(std::move(column_name)) {}

   StringColumnMetadata(std::string column_name, silo::preprocessing::PhyloTreeFile phylo_tree)
       : ColumnMetadata(std::move(column_name)),
         phylo_tree(std::move(phylo_tree)) {}

   StringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalMap<std::string>&& dictionary
   )
       : ColumnMetadata(std::move(column_name)),
         dictionary(std::move(dictionary)) {}

   StringColumnMetadata(
      std::string column_name,
      silo::common::BidirectionalMap<std::string>&& dictionary,
      silo::preprocessing::PhyloTreeFile phylo_tree
   )
       : ColumnMetadata(std::move(column_name)),
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

   YAML::Node toYAML() const override;
   static std::shared_ptr<StringColumnMetadata> fromYAML(
      std::string column_name,
      const YAML::Node& yaml_node
   );
};

class StringColumnPartition {
  public:
   using Metadata = StringColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::STRING;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<common::String<silo::common::STRING_SIZE>> values;
   Metadata* metadata;

  public:
   explicit StringColumnPartition(Metadata* metadata);

   [[nodiscard]] const std::vector<common::String<silo::common::STRING_SIZE>>& getValues() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] std::optional<common::String<silo::common::STRING_SIZE>> embedString(
      const std::string& string
   ) const;

   [[nodiscard]] inline std::string lookupValue(common::String<silo::common::STRING_SIZE> string
   ) const {
      return string.toString(metadata->dictionary);
   }
};

}  // namespace silo::storage::column
