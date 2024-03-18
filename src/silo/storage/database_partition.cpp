#include "silo/storage/database_partition.h"

#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column_group.h"
#include "silo/storage/sequence_store.h"

namespace silo {
namespace storage::column {
class DateColumnPartition;
class FloatColumnPartition;
class IndexedStringColumnPartition;
class IntColumnPartition;
class PangoLineageColumnPartition;
class StringColumnPartition;
template <typename SymbolType>
class InsertionColumnPartition;
}  // namespace storage::column

DatabasePartition::DatabasePartition(std::vector<silo::preprocessing::PartitionChunk> chunks)
    : chunks(std::move(chunks)) {}

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

void DatabasePartition::validateMetadataColumns() const {
   const size_t partition_size = sequence_count;

   for (const auto& col : columns.aa_insertion_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "aa_insertion_column " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.pango_lineage_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "pango_lineage_column " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.nuc_insertion_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "nuc_insertion_column " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.date_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "date_column " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.bool_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "bool_columns " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.int_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "int_columns " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.indexed_string_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "indexed_string_columns " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.string_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "string_columns " + col.first + " has invalid size."
         );
      }
   }
   for (const auto& col : columns.float_columns) {
      if (col.second.getValues().size() != partition_size) {
         throw preprocessing::PreprocessingException(
            "float_columns " + col.first + " has invalid size."
         );
      }
   }
}

const std::vector<preprocessing::PartitionChunk>& DatabasePartition::getChunks() const {
   return chunks;
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
   storage::column::InsertionColumnPartition<Nucleotide>& column
) {
   columns.nuc_insertion_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::InsertionColumnPartition<AminoAcid>& column
) {
   columns.aa_insertion_columns.insert({std::string(name), column});
}

void DatabasePartition::insertColumn(
   const std::string& name,
   storage::column::PangoLineageColumnPartition& column
) {
   columns.pango_lineage_columns.insert({std::string(name), column});
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
