#include "silo/storage/table_partition.h"

#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::storage {

void TablePartition::validate() const {
   validateNucleotideSequences();
   validateAminoAcidSequences();
   validateMetadataColumns();
}

void TablePartition::finalize() {
   for (auto& [_, sequence_column] : columns.nuc_columns) {
      sequence_column.finalize();
   }
   for (auto& [_, sequence_column] : columns.aa_columns) {
      sequence_column.finalize();
   }
}

TablePartition::TablePartition(schema::TableSchema& schema) {
   auto column_initializer = []<column::Column ColumnType>(
                                ColumnPartitionGroup& column_group,
                                const schema::ColumnIdentifier& column_identifier,
                                schema::TableSchema& schema
                             ) {
      ColumnType column(schema.getColumnMetadata<ColumnType>(column_identifier.name).value());
      column_group.metadata.emplace_back(column_identifier);
      column_group.getColumns<ColumnType>().emplace(column_identifier.name, column);
   };
   for (const auto& column : schema.getColumnIdentifiers()) {
      column::visit(column.type, column_initializer, columns, column, schema);
   }
}

void TablePartition::validateNucleotideSequences() const {
   const size_t partition_size = sequence_count;

   for (const auto& [name, nuc_column] : columns.nuc_columns) {
      if (nuc_column.sequence_count > partition_size) {
         SILO_PANIC(
            "nuc_store {} ({}) has invalid size (expected {}).",
            name,
            nuc_column.sequence_count,
            partition_size
         );
      }
      if (nuc_column.positions.size() != nuc_column.metadata->reference_sequence.size()) {
         SILO_PANIC(
            "nuc_store positions {} ({}) has size unequal to reference (expected {}).",
            name,
            nuc_column.positions.size(),
            nuc_column.metadata->reference_sequence.size()
         );
      }
      if (nuc_column.metadata->reference_sequence.empty()) {
         SILO_PANIC("reference_sequence {} is empty.", name);
      }
      if (nuc_column.missing_symbol_bitmaps.size() > partition_size) {
         SILO_PANIC("nuc_store.missing_symbol_bitmaps {} has invalid size.", name);
      }
   }
}

void TablePartition::validateAminoAcidSequences() const {
   const size_t partition_size = sequence_count;

   for (const auto& [name, aa_column] : columns.aa_columns) {
      if (aa_column.sequence_count > partition_size) {
         SILO_PANIC(
            "aa_store {} ({}) has invalid size (expected {}).",
            name,
            aa_column.sequence_count,
            partition_size
         );
      }
      if (aa_column.positions.size() != aa_column.metadata->reference_sequence.size()) {
         SILO_PANIC("aa_store {} has invalid position size.", name);
      }
      if (aa_column.metadata->reference_sequence.empty()) {
         SILO_PANIC("reference_sequence {} is empty.", name);
      }
      if (aa_column.missing_symbol_bitmaps.size() > partition_size) {
         SILO_PANIC("aa_store.missing_symbol_bitmaps {} has invalid size.", name);
      }
   }
}

template <typename ColumnPartition>
void TablePartition::validateColumnsHaveSize(
   const std::map<std::string, ColumnPartition>& columnsOfTheType,
   const std::string& columnType
) const {
   for (const auto& column : columnsOfTheType) {
      if (column.second.getValues().size() != sequence_count) {
         throw preprocessing::PreprocessingException(
            columnType + " " + column.first + " has invalid size " +
            std::to_string(column.second.getValues().size())
         );
      }
   }
}

void TablePartition::validateMetadataColumns() const {
   validateColumnsHaveSize(columns.date_columns, "date_column");
   validateColumnsHaveSize(columns.bool_columns, "bool_columns");
   validateColumnsHaveSize(columns.int_columns, "int_columns");
   validateColumnsHaveSize(columns.indexed_string_columns, "indexed_string_columns");
   validateColumnsHaveSize(columns.string_columns, "string_columns");
   validateColumnsHaveSize(columns.float_columns, "float_columns");
}

}  // namespace silo::storage
