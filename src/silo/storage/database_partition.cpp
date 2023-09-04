#include "silo/storage/database_partition.h"

#include <utility>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/concurrent_vector.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/preprocessing/partition.h"
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

DatabasePartition::DatabasePartition(std::vector<silo::preprocessing::Chunk> chunks)
    : chunks(std::move(chunks)) {}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void DatabasePartition::flipBitmaps() {
   for (auto& [_, seq_store] : nuc_sequences) {
      tbb::enumerable_thread_specific<decltype(seq_store.indexing_differences_to_reference_sequence
      )>
         flipped_bitmaps;

      auto& positions = seq_store.positions;
      tbb::parallel_for(tbb::blocked_range<uint32_t>(0, positions.size()), [&](const auto& local) {
         auto& local_flipped_bitmaps = flipped_bitmaps.local();
         for (auto position = local.begin(); position != local.end(); ++position) {
            auto flipped_symbol = positions[position].flipMostNumerousBitmap(sequence_count);
            if (flipped_symbol.has_value()) {
               local_flipped_bitmaps.emplace_back(position, *flipped_symbol);
            }
         }
      });
      for (const auto& local : flipped_bitmaps) {
         for (const auto& element : local) {
            seq_store.indexing_differences_to_reference_sequence.emplace_back(element);
         }
      }
   }
   for (auto& [_, aa_store] : aa_sequences) {
      tbb::enumerable_thread_specific<decltype(aa_store.indexing_differences_to_reference_sequence)>
         flipped_bitmaps;

      auto& positions = aa_store.positions;
      tbb::parallel_for(tbb::blocked_range<uint32_t>(0, positions.size()), [&](const auto& local) {
         auto& local_flipped_bitmaps = flipped_bitmaps.local();
         for (auto position = local.begin(); position != local.end(); ++position) {
            auto flipped_symbol = positions[position].flipMostNumerousBitmap(sequence_count);
            if (flipped_symbol.has_value()) {
               local_flipped_bitmaps.emplace_back(position, *flipped_symbol);
            }
         }
      });
      for (const auto& local : flipped_bitmaps) {
         for (const auto& element : local) {
            aa_store.indexing_differences_to_reference_sequence.emplace_back(element);
         }
      }
   }
}

const std::vector<preprocessing::Chunk>& DatabasePartition::getChunks() const {
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
