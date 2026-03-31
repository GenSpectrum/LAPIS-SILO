#include "silo/storage/table.h"

#include <fstream>
#include <unordered_set>
#include <utility>

#include <spdlog/spdlog.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "evobench/evobench.hpp"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/roaring_util/roaring_serialize.h"
#include "silo/schema/duplicate_primary_key_exception.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::storage {

Table::Table(std::shared_ptr<schema::TableSchema> schema)
    : schema(std::move(schema)) {
   auto column_initializer = []<column::Column ColumnType>(
                                ColumnPartitionGroup& column_group,
                                const silo::schema::ColumnIdentifier& column_identifier,
                                silo::schema::TableSchema& table_schema
                             ) {
      ColumnType column(table_schema.getColumnMetadata<ColumnType>(column_identifier.name).value());
      column_group.metadata.emplace_back(column_identifier);
      column_group.getColumns<ColumnType>().emplace(column_identifier.name, std::move(column));
   };
   for (const auto& col : this->schema->getColumnIdentifiers()) {
      column::visit(col.type, column_initializer, columns, col, *this->schema);
   }
}

void Table::validate() const {
   validatePrimaryKeyUnique();
   validateNucleotideSequences();
   validateAminoAcidSequences();
   validateMetadataColumns();
}

void Table::finalize() {
   for (auto& [_, sequence_column] : columns.nuc_columns) {
      sequence_column.finalize();
   }
   for (auto& [_, sequence_column] : columns.aa_columns) {
      sequence_column.finalize();
   }
}

void Table::validatePrimaryKeyUnique() const {
   SPDLOG_DEBUG("Checking that primary keys are unique.");
   const auto primary_key = schema->primary_key;
   SILO_ASSERT(primary_key.type == schema::ColumnType::STRING);

   const auto& primary_key_column = columns.string_columns.at(primary_key.name);
   auto num_values = primary_key_column.numValues();

   std::unordered_set<std::string> unique_keys;
   unique_keys.reserve(num_values);
   for (size_t i = 0; i < num_values; ++i) {
      std::string value = primary_key_column.getValueString(i);
      if (unique_keys.contains(value)) {
         throw schema::DuplicatePrimaryKeyException("Found duplicate primary key {}", value);
      }
      unique_keys.insert(value);
   }
   SPDLOG_DEBUG("Found {} distinct primary keys.", unique_keys.size());
}

void Table::validateNucleotideSequences() const {
   for (const auto& [name, nuc_column] : columns.nuc_columns) {
      if (nuc_column.sequence_count > sequence_count) {
         SILO_PANIC(
            "nuc_store {} ({}) has invalid size (expected {}).",
            name,
            nuc_column.sequence_count,
            sequence_count
         );
      }
      if (nuc_column.metadata->reference_sequence.empty()) {
         SILO_PANIC("reference_sequence {} is empty.", name);
      }
   }
}

void Table::validateAminoAcidSequences() const {
   for (const auto& [name, aa_column] : columns.aa_columns) {
      if (aa_column.sequence_count > sequence_count) {
         SILO_PANIC(
            "aa_store {} ({}) has invalid size (expected {}).",
            name,
            aa_column.sequence_count,
            sequence_count
         );
      }
      if (aa_column.metadata->reference_sequence.empty()) {
         SILO_PANIC("reference_sequence {} is empty.", name);
      }
   }
}

template <typename ColumnPartition>
void Table::validateColumnsHaveSize(
   const std::map<std::string, ColumnPartition>& columnsOfTheType,
   const std::string& columnType
) const {
   for (const auto& col : columnsOfTheType) {
      if (col.second.numValues() != sequence_count) {
         throw preprocessing::PreprocessingException(
            columnType + " " + col.first + " has invalid size " +
            std::to_string(col.second.numValues())
         );
      }
   }
}

void Table::validateMetadataColumns() const {
   validateColumnsHaveSize(columns.date32_columns, "date32_columns");
   validateColumnsHaveSize(columns.bool_columns, "bool_columns");
   validateColumnsHaveSize(columns.int_columns, "int_columns");
   validateColumnsHaveSize(columns.indexed_string_columns, "indexed_string_columns");
   validateColumnsHaveSize(columns.string_columns, "string_columns");
   validateColumnsHaveSize(columns.float_columns, "float_columns");
}

namespace {

std::ifstream openInputFileOrThrow(const std::string& path) {
   std::ifstream file(path, std::ios::binary);
   if (!file) {
      auto error = fmt::format("Input file {} could not be opened.", path);
      throw persistence::LoadDatabaseException(error);
   }
   return file;
}

std::ofstream openOutputFileOrThrow(const std::string& path) {
   std::ofstream file(path, std::ios::binary);
   if (!file) {
      auto error = fmt::format("Output file {} could not be opened.", path);
      throw persistence::SaveDatabaseException(error);
   }
   return file;
}

}  // namespace

void Table::saveData(const std::filesystem::path& path) {
   EVOBENCH_SCOPE("Table", "saveData");
   auto output_file = openOutputFileOrThrow(path);
   if (!output_file) {
      throw persistence::SaveDatabaseException(
         "Cannot open output file " + path.string() + " for saving"
      );
   }

   SPDLOG_INFO("Saving table data...");
   ::boost::archive::binary_oarchive output_archive(output_file);
   serializeData(output_archive, 0);
   SPDLOG_INFO("Finished saving table data");
}

void Table::loadData(const std::filesystem::path& path) {
   EVOBENCH_SCOPE("Table", "loadData");

   auto input_file = openInputFileOrThrow(path);
   ::boost::archive::binary_iarchive input_archive(input_file);
   serializeData(input_archive, 0);
   SPDLOG_INFO("Finished loading table data");
}

}  // namespace silo::storage
