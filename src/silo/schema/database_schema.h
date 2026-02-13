#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

#include <spdlog/spdlog.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include "silo/common/panic.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::schema {

enum class ColumnType : uint8_t {
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
   SILO_UNREACHABLE();
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

   [[nodiscard]] std::optional<ColumnIdentifier> getColumn(std::string_view name) const;

   [[nodiscard]] std::vector<ColumnIdentifier> getColumnIdentifiers() const;

   template <typename SymbolType>
   [[nodiscard]] std::optional<ColumnIdentifier> getDefaultSequenceName() const;

   template <silo::storage::column::Column ColumnType>
   [[nodiscard]] std::vector<ColumnIdentifier> getColumnByType() const {
      std::vector<ColumnIdentifier> result;
      for (const auto& [column_identifier, _] : column_metadata) {
         if (column_identifier.type == ColumnType::TYPE) {
            result.push_back(column_identifier);
         }
      }
      return result;
   }

   template <storage::column::Column ColumnType>
   [[nodiscard]] std::optional<typename ColumnType::Metadata*> getColumnMetadata(
      std::string_view name
   ) const {
      auto iter = std::ranges::find_if(column_metadata, [&name](const auto& metadata_pair) {
         return metadata_pair.first.name == name;
      });
      if (iter == column_metadata.end() || iter->first.type != ColumnType::TYPE) {
         SPDLOG_INFO(
            "Mismatching type found: expected {} vs actual {}",
            columnTypeToString(ColumnType::TYPE),
            columnTypeToString(iter->first.type)
         );
         return std::nullopt;
      }
      auto typed_metadata = dynamic_cast<typename ColumnType::Metadata*>(iter->second.get());
      SILO_ASSERT(typed_metadata != nullptr);
      return typed_metadata;
   }

   TableSchema() = default;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void save(Archive& archive, unsigned int version) const;

   template <class Archive>
   void load(Archive& archive, unsigned int version);

   template <class Archive>
   void serialize(Archive& archive, const unsigned int version) {
      boost::serialization::split_member(archive, *this, version);
   }
};

// NOLINTNEXTLINE(bugprone-exception-escape): false positive in libc++ pair with llvm
class TableName {
   std::string name;

  public:
   explicit TableName(std::string name);

   [[nodiscard]] const std::string& getName() const { return name; }

   static const TableName& getDefault();

   bool operator==(const TableName& other) const { return name == other.name; }
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

   [[nodiscard]] const TableSchema& getDefaultTableSchema() const;

   static DatabaseSchema loadFromFile(const std::filesystem::path& file_path);
   void saveToFile(const std::filesystem::path& file_path) const;
};

}  // namespace silo::schema
