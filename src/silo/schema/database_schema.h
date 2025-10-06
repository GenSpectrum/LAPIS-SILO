#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include "silo/common/panic.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::schema {

enum class ColumnType {
   STRING,
   INDEXED_STRING,
   DATE,
   BOOL,
   INT32,
   FLOAT,
   AMINO_ACID_SEQUENCE,
   NUCLEOTIDE_SEQUENCE,
   ZSTD_COMPRESSED_STRING,
   INT64
};

constexpr std::string_view columnTypeToString(ColumnType type) {
   switch (type) {
      case ColumnType::STRING:
         return "STRING";
      case ColumnType::INDEXED_STRING:
         return "INDEXED_STRING";
      case ColumnType::DATE:
         return "DATE";
      case ColumnType::BOOL:
         return "BOOL";
      case ColumnType::INT32:
         return "INT32";
      case ColumnType::INT64:
         return "INT64";
      case ColumnType::FLOAT:
         return "FLOAT";
      case ColumnType::AMINO_ACID_SEQUENCE:
         return "AMINO_ACID_SEQUENCE";
      case ColumnType::NUCLEOTIDE_SEQUENCE:
         return "NUCLEOTIDE_SEQUENCE";
      case ColumnType::ZSTD_COMPRESSED_STRING:
         return "ZSTD_COMPRESSED_STRING";
   }
}

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

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      archive & name;
      archive & type;
   }
};

class TableSchema {
  public:
   std::map<ColumnIdentifier, std::shared_ptr<storage::column::ColumnMetadata>> column_metadata;
   std::optional<ColumnIdentifier> default_nucleotide_sequence;
   std::optional<ColumnIdentifier> default_aa_sequence;
   ColumnIdentifier primary_key;

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
      auto typed_metadata = dynamic_cast<typename ColumnType::Metadata*>(it->second.get());
      SILO_ASSERT(typed_metadata != nullptr);
      return typed_metadata;
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
      auto typed_metadata = dynamic_cast<typename ColumnType::Metadata*>(it->second.get());
      SILO_ASSERT(typed_metadata != nullptr);
      return typed_metadata;
   }

   TableSchema() = default;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void save(Archive& ar, const unsigned int version) const;

   template <class Archive>
   void load(Archive& ar, const unsigned int version);

   template <class Archive>
   void serialize(Archive& ar, const unsigned int version) {
      boost::serialization::split_member(ar, *this, version);
   }
};

class TableName {
   std::string name;

  public:
   TableName(std::string_view name);

   const std::string& getName() const { return name; }

   static const TableName& getDefault();

   bool operator<(const TableName& other) const { return name < other.name; }

   TableName() = default;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      archive & name;
   }
};

class DatabaseSchema {
  public:
   std::map<TableName, TableSchema> tables;

   const TableSchema& getDefaultTableSchema() const;

   static DatabaseSchema loadFromFile(const std::filesystem::path& file_path);
   void saveToFile(const std::filesystem::path& file_path);
};

}  // namespace silo::schema
