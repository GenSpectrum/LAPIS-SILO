#include "silo/storage/database_partition.h"

#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo {

void DatabasePartition::validate() const {
   validateNucleotideSequences();
   validateAminoAcidSequences();
   validateMetadataColumns();
}

void DatabasePartition::validateNucleotideSequences() const {
   const size_t partition_size = sequence_count;

   for (const auto& [name, nuc_store] : nuc_sequences) {
      if (nuc_store.sequence_count > partition_size) {
         throw preprocessing::PreprocessingException(fmt::format(
            "nuc_store {} ({}) has invalid size (expected {}).",
            name,
            nuc_store.sequence_count,
            partition_size
         ));
      }
      if (nuc_store.positions.size() != nuc_store.reference_sequence.size()) {
         throw preprocessing::PreprocessingException(fmt::format(
            "nuc_store positions {} ({}) has size unequal to reference (expected {}).",
            name,
            nuc_store.positions.size(),
            nuc_store.reference_sequence.size()
         ));
      }
      if (nuc_store.reference_sequence.empty()) {
         throw preprocessing::PreprocessingException("reference_sequence " + name + " is empty.");
      }
      if (nuc_store.missing_symbol_bitmaps.size() > partition_size) {
         throw preprocessing::PreprocessingException(
            "nuc_store.missing_symbol_bitmaps " + name + " has invalid size."
         );
      }
   }
}

void DatabasePartition::validateAminoAcidSequences() const {
   const size_t partition_size = sequence_count;

   for (const auto& [name, aa_store] : aa_sequences) {
      if (aa_store.sequence_count > partition_size) {
         throw preprocessing::PreprocessingException(fmt::format(
            "aa_store {} ({}) has invalid size (expected {}).",
            name,
            aa_store.sequence_count,
            partition_size
         ));
      }
      if (aa_store.positions.size() != aa_store.reference_sequence.size()) {
         throw preprocessing::PreprocessingException(
            "aa_store " + name + " has invalid position size."
         );
      }
      if (aa_store.reference_sequence.empty()) {
         throw preprocessing::PreprocessingException("reference_sequence " + name + " is empty.");
      }
      if (aa_store.missing_symbol_bitmaps.size() > partition_size) {
         throw preprocessing::PreprocessingException(
            "aa_store.missing_symbol_bitmaps " + name + " has invalid size."
         );
      }
   }
}

template <typename ColumnPartition>
void DatabasePartition::validateColumnsHaveSize(
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

void DatabasePartition::validateMetadataColumns() const {
   validateColumnsHaveSize(columns.date_columns, "date_column");
   validateColumnsHaveSize(columns.bool_columns, "bool_columns");
   validateColumnsHaveSize(columns.int_columns, "int_columns");
   validateColumnsHaveSize(columns.indexed_string_columns, "indexed_string_columns");
   validateColumnsHaveSize(columns.string_columns, "string_columns");
   validateColumnsHaveSize(columns.float_columns, "float_columns");
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::StringColumnPartition& column
) {
   columns.string_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IndexedStringColumnPartition& column
) {
   columns.indexed_string_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::BoolColumnPartition& column
) {
   columns.bool_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::IntColumnPartition& column
) {
   columns.int_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::DateColumnPartition& column
) {
   columns.date_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::FloatColumnPartition& column
) {
   columns.float_columns.insert({std::string(name), column});
}

template <>
const std::map<std::string, SequenceStorePartition<Nucleotide>&>& DatabasePartition::
   getSequenceStores<Nucleotide>() const {
   return nuc_sequences;
}

template <>
const std::map<std::string, SequenceStorePartition<AminoAcid>&>& DatabasePartition::
   getSequenceStores<AminoAcid>() const {
   return aa_sequences;
}

}  // namespace silo
