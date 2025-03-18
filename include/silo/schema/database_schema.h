#pragma once

#include <yaml-cpp/node/node.h>
#include <map>
#include <optional>
#include <vector>

#include "silo/common/panic.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::schema {

enum class ColumnType {
   STRING,
   INDEXED_STRING,
   DATE,
   BOOL,
   INT,
   FLOAT,
   AMINO_ACID_SEQUENCE,
   NUCLEOTIDE_SEQUENCE,
   ZSTD_COMPRESSED_STRING
};

bool isSequenceColumn(ColumnType type);

struct ColumnIdentifier {
   std::string name;
   silo::schema::ColumnType type;

   bool operator<(const ColumnIdentifier& other) const {
      return std::tie(name, type) < std::tie(other.name, other.type);
   }

   bool operator==(const ColumnIdentifier& other) const {
      return std::tie(name, type) == std::tie(other.name, other.type);
   }
};

class TableSchema {
   std::map<ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>> column_metadata;

  public:
   std::optional<ColumnIdentifier> default_nucleotide_sequence;
   std::optional<ColumnIdentifier> default_aa_sequence;
   const ColumnIdentifier primary_key;

   TableSchema(
      std::map<ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>> column_metadata,
      ColumnIdentifier primary_key
   )
       : column_metadata(std::move(column_metadata)),
         primary_key(std::move(primary_key)) {
      SILO_ASSERT(this->column_metadata.contains(this->primary_key));
   }

   std::optional<ColumnIdentifier> getColumn(std::string_view name) const;

   std::vector<ColumnIdentifier> getColumnIdentifiers() const;

   template <typename SymbolType>
   std::optional<ColumnIdentifier> getDefaultSequenceName() const;

   YAML::Node toYAML() const;

   static TableSchema fromYAML(const YAML::Node& yaml);

   template <silo::storage::column::Column ColumnType>
   std::vector<ColumnIdentifier> getColumnByType() const {
      std::vector<ColumnIdentifier> result;
      for (const auto& [column_identifier, _] : column_metadata) {
         if (column_identifier.type == ColumnType::TYPE) {
            result.push_back(column_identifier);
         }
      }
      return result;
   }

   template <storage::column::Column ColumnType>
   std::optional<typename ColumnType::Metadata*> getColumnMetadata(std::string_view name) {
      auto it = std::ranges::find_if(column_metadata, [&name](const auto& metadata_pair) {
         return metadata_pair.first.name == name;
      });
      if (it == column_metadata.end() || it->first.type != ColumnType::TYPE) {
         return std::nullopt;
      }
      return dynamic_cast<typename ColumnType::Metadata*>(it->second.get());
   }

   template <storage::column::Column ColumnType>
   const std::optional<typename ColumnType::Metadata*> getColumnMetadata(std::string_view name
   ) const {
      auto it = std::ranges::find_if(column_metadata, [&name](const auto& metadata_pair) {
         return metadata_pair.first.name == name;
      });
      if (it == column_metadata.end() || it->first.type != ColumnType::TYPE) {
         return std::nullopt;
      }
      return dynamic_cast<typename ColumnType::Metadata*>(it->second.get());
   }
};

class TableName {
   std::string name;

  public:
   TableName(std::string_view name);

   const std::string& getName() const { return name; }

   static const TableName& getDefault();

   bool operator<(const TableName& other) const { return name < other.name; }
};

class DatabaseSchema {
  public:
   std::map<TableName, TableSchema> tables;

   YAML::Node toYAML() const;
   static DatabaseSchema fromYAML(const YAML::Node& yaml);

   const TableSchema& getDefaultTableSchema() const;
};

}  // namespace silo::schema
