#include "silo/schema/database_schema.h"

#include <fstream>
#include <optional>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "silo/storage/column/column_metadata.h"
#include "silo/storage/column/column_type_visitor.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::schema {

bool isSequenceColumn(ColumnType type) {
   return type == ColumnType::NUCLEOTIDE_SEQUENCE || type == ColumnType::AMINO_ACID_SEQUENCE ||
          type == ColumnType::ZSTD_COMPRESSED_STRING;
}

std::optional<ColumnIdentifier> TableSchema::getColumn(std::string_view name) const {
   auto it = std::ranges::find_if(column_metadata, [&name](const auto& metadata_pair) {
      return metadata_pair.first.name == name;
   });
   if (it == column_metadata.end()) {
      return std::nullopt;
   }
   return it->first;
}

std::vector<ColumnIdentifier> TableSchema::getColumnIdentifiers() const {
   std::vector<ColumnIdentifier> result;
   for (const auto& [column_identifier, _] : column_metadata) {
      result.push_back(column_identifier);
   }
   return result;
}

template <>
std::optional<ColumnIdentifier> TableSchema::getDefaultSequenceName<Nucleotide>() const {
   return default_nucleotide_sequence;
}

template <>
std::optional<ColumnIdentifier> TableSchema::getDefaultSequenceName<AminoAcid>() const {
   return default_aa_sequence;
}

class ColumnMetadataSaverByType {
  public:
   template <storage::column::Column ColumnType, class Archive>
   void operator()(Archive& archive, std::shared_ptr<storage::column::ColumnMetadata> metadata) {
      auto typed_metadata = dynamic_cast<typename ColumnType::Metadata*>(metadata.get());
      SILO_ASSERT(typed_metadata != nullptr);
      archive << *typed_metadata;
   }
};

class ColumnMetadataLoaderByType {
  public:
   template <storage::column::Column ColumnType, class Archive>
   std::shared_ptr<storage::column::ColumnMetadata> operator()(Archive& archive) {
      std::shared_ptr<typename ColumnType::Metadata> metadata;
      archive >> metadata;
      return metadata;
   }
};

template <class Archive>
void TableSchema::save(Archive& archive, const unsigned int version) const {
   archive & default_nucleotide_sequence;
   archive & default_aa_sequence;
   archive & primary_key;

   std::vector<ColumnIdentifier> column_identifiers;
   for (const auto& [column_identifier, _] : column_metadata) {
      column_identifiers.push_back(column_identifier);
   }
   archive & column_identifiers;

   for (const auto& [column_identifier, metadata] : column_metadata) {
      storage::column::visit(
         column_identifier.type, ColumnMetadataSaverByType{}, archive, metadata
      );
   }
}

template <class Archive>
void TableSchema::load(Archive& archive, const unsigned int version) {
   archive & default_nucleotide_sequence;
   archive & default_aa_sequence;
   archive & primary_key;

   std::vector<ColumnIdentifier> column_identifiers;
   archive & column_identifiers;

   for (const auto& column_identifier : column_identifiers) {
      column_metadata.emplace(
         column_identifier,
         storage::column::visit(column_identifier.type, ColumnMetadataLoaderByType{}, archive)
      );
   }
}

TableName::TableName(std::string_view name) {
   for (char c : name) {
      if (c < 'a' || c > 'z') {
         throw std::runtime_error("Table names may only contain lower-case letters");
      }
   }
   this->name = name;
}

TableName default_table_name{"default"};

const TableName& TableName::getDefault() {
   return default_table_name;
}

const TableSchema& DatabaseSchema::getDefaultTableSchema() const {
   return tables.at(TableName::getDefault());
}

}  // namespace silo::schema

namespace silo::schema {

void DatabaseSchema::saveToFile(const std::filesystem::path& file_path) {
   std::ofstream database_schema_file{file_path, std::ios::binary};
   boost::archive::binary_oarchive output_archive(database_schema_file);
   output_archive << tables;
}

DatabaseSchema DatabaseSchema::loadFromFile(const std::filesystem::path& file_path) {
   std::ifstream database_schema_file{file_path, std::ios::binary};
   boost::archive::binary_iarchive input_archive(database_schema_file);
   schema::DatabaseSchema schema;
   input_archive >> schema.tables;
   return schema;
}

}  // namespace silo::schema
