#include "silo/storage/sequence_store.h"

#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/position.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

template <typename SymbolType>
silo::SequenceStorePartition<SymbolType>::SequenceStorePartition(
   const std::vector<typename SymbolType::Symbol>& reference_sequence
)
    : reference_sequence(reference_sequence) {
   positions.reserve(reference_sequence.size());
   for (const auto symbol : reference_sequence) {
      positions.emplace_back(Position<SymbolType>::fromInitiallyFlipped(symbol));
   }
}

template <typename SymbolType>
size_t silo::SequenceStorePartition<SymbolType>::fill(ZstdFastaTableReader& input) {
   static constexpr size_t BUFFER_SIZE = 1024;

   input.loadTable();

   size_t read_sequences_count = 0;

   std::vector<std::optional<std::string>> genome_buffer;

   std::optional<std::string> key;
   std::optional<std::string> genome;
   while (true) {
      key = input.next(genome);
      if (!key) {
         break;
      }
      genome_buffer.push_back(std::move(genome));
      if (genome_buffer.size() >= BUFFER_SIZE) {
         interpret(genome_buffer);
         genome_buffer.clear();
      }

      ++read_sequences_count;
   }
   interpret(genome_buffer);
   const SequenceStoreInfo info_before_optimisation = getInfo();

   SPDLOG_DEBUG(
      "Sequence store partition info after filling it: {}, and after optimising: {}",
      info_before_optimisation,
      getInfo()
   );

   return read_sequences_count;
}

namespace {

constexpr std::string_view DELIMITER_INSERTION = ":";

struct InsertionEntry {
   uint32_t position_idx;
   std::string insertion;
};

InsertionEntry parseInsertion(const std::string& value) {
   auto position_and_insertion = silo::splitBy(value, DELIMITER_INSERTION);
   std::transform(
      position_and_insertion.begin(),
      position_and_insertion.end(),
      position_and_insertion.begin(),
      [](const std::string& value) { return silo::removeSymbol(value, '\"'); }
   );
   try {
      if (position_and_insertion.size() == 2) {
         const auto position = boost::lexical_cast<uint32_t>(position_and_insertion[0]);
         const auto& insertion = position_and_insertion[1];
         return {.position_idx = position, .insertion = insertion};
      }
   } catch (const boost::bad_lexical_cast& error) {
      const std::string message = "Failed to parse insertion due to invalid format: " + value;
      throw silo::preprocessing::PreprocessingException(message + ". Error: " + error.what());
   }

   const std::string message = "Failed to parse insertion due to invalid format: " + value;
   throw silo::preprocessing::PreprocessingException(message);
}
}  // namespace

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::insertInsertion(
   size_t row_id,
   const std::string& insertion_and_position
) {
   auto [position, insertion] = parseInsertion(insertion_and_position);
   insertion_index.addLazily(position, insertion, row_id);
}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::finalize() {
   SPDLOG_DEBUG("Building insertion index");

   insertion_index.buildIndex();

   SPDLOG_DEBUG("Optimizing bitmaps");

   const SequenceStoreInfo info_before_optimisation = getInfo();
   optimizeBitmaps();

   SPDLOG_DEBUG(
      "Sequence store partition info after filling it: {}, and after optimising: {}",
      info_before_optimisation,
      getInfo()
   );
}

[[maybe_unused]] auto fmt::formatter<silo::SequenceStoreInfo>::format(
   const silo::SequenceStoreInfo& sequence_store_info,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return fmt::format_to(
      ctx.out(),
      "SequenceStoreInfo[sequence count: {}, size: {}, N bitmaps size: {}]",
      sequence_store_info.sequence_count,
      sequence_store_info.size,
      silo::formatNumber(sequence_store_info.n_bitmaps_size)
   );
}

template <typename SymbolType>
silo::SequenceStoreInfo silo::SequenceStorePartition<SymbolType>::getInfo() const {
   size_t n_bitmaps_size = 0;
   for (const auto& bitmap : missing_symbol_bitmaps) {
      n_bitmaps_size += bitmap.getSizeInBytes(false);
   }
   return SequenceStoreInfo{this->sequence_count, computeSize(), n_bitmaps_size};
}

template <typename SymbolType>
const roaring::Roaring* silo::SequenceStorePartition<SymbolType>::getBitmap(
   size_t position_idx,
   typename SymbolType::Symbol symbol
) const {
   return positions[position_idx].getBitmap(symbol);
}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::fillIndexes(
   const std::vector<std::optional<std::string>>& genomes
) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR),
      [&](const auto& local) {
         SymbolMap<SymbolType, std::vector<uint32_t>> ids_per_symbol_for_current_position;
         for (size_t position_idx = local.begin(); position_idx != local.end(); ++position_idx) {
            const size_t number_of_sequences = genomes.size();
            for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
               const auto& genome = genomes[sequence_id];
               if (!genome.has_value()) {
                  continue;
               }
               const char character = genome.value()[position_idx];
               const auto symbol = SymbolType::charToSymbol(character);
               if (!symbol.has_value()) {
                  throw silo::preprocessing::PreprocessingException(
                     "Illegal character " + std::to_string(character) + " contained in sequence."
                  );
               }
               if (symbol != SymbolType::SYMBOL_MISSING) {
                  ids_per_symbol_for_current_position[*symbol].push_back(
                     sequence_count + sequence_id
                  );
               }
            }
            addSymbolsToPositions(
               position_idx, ids_per_symbol_for_current_position, number_of_sequences
            );
         }
      }
   );
}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::addSymbolsToPositions(
   size_t position_idx,
   SymbolMap<SymbolType, std::vector<uint32_t>>& ids_per_symbol_for_current_position,
   size_t number_of_sequences
) {
   for (const auto& symbol : SymbolType::SYMBOLS) {
      positions[position_idx].addValues(
         symbol, ids_per_symbol_for_current_position.at(symbol), sequence_count, number_of_sequences
      );
      ids_per_symbol_for_current_position[symbol].clear();
   }
}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::fillNBitmaps(
   const std::vector<std::optional<std::string>>& genomes
) {
   const size_t genome_length = positions.size();

   missing_symbol_bitmaps.resize(sequence_count + genomes.size());

   const tbb::blocked_range<size_t> range(0, genomes.size());
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      std::vector<uint32_t> positions_with_symbol_missing;
      for (size_t sequence_index = local.begin(); sequence_index != local.end(); ++sequence_index) {
         const auto& maybe_genome = genomes[sequence_index];

         if (!maybe_genome.has_value()) {
            missing_symbol_bitmaps[sequence_count + sequence_index].addRange(0, genome_length);
            missing_symbol_bitmaps[sequence_count + sequence_index].runOptimize();
            continue;
         }

         const auto& genome = maybe_genome.value();

         for (size_t position_idx = 0; position_idx < genome_length; ++position_idx) {
            const char character = genome[position_idx];
            const auto symbol = SymbolType::charToSymbol(character);
            if (symbol == SymbolType::SYMBOL_MISSING) {
               positions_with_symbol_missing.push_back(position_idx);
            }
         }
         if (!positions_with_symbol_missing.empty()) {
            missing_symbol_bitmaps[sequence_count + sequence_index].addMany(
               positions_with_symbol_missing.size(), positions_with_symbol_missing.data()
            );
            missing_symbol_bitmaps[sequence_count + sequence_index].runOptimize();
            positions_with_symbol_missing.clear();
         }
      }
   });
}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::optimizeBitmaps() {
   size_t size_of_positions = computeSize();

   tbb::enumerable_thread_specific<decltype(indexing_differences_to_reference_sequence)>
      index_changes_to_reference;

   tbb::parallel_for(tbb::blocked_range<uint32_t>(0, positions.size()), [&](const auto& local) {
      auto& local_index_changes = index_changes_to_reference.local();
      for (auto position_idx = local.begin(); position_idx != local.end(); ++position_idx) {
         auto symbol_changed = positions[position_idx].flipMostNumerousBitmap(sequence_count);
         if (symbol_changed.has_value()) {
            local_index_changes.emplace_back(position_idx, *symbol_changed);
         }
         auto highest_symbol_result = positions[position_idx].getHighestCardinalitySymbol(sequence_count);
         if (highest_symbol_result.has_value()) {
            const size_t logicalCardinality = highest_symbol_result.value().second;
            if (static_cast<double_t>(logicalCardinality) / sequence_count < 0.1) {
               symbol_changed = positions[position_idx].deleteMostNumerousBitmap(sequence_count);
               if (symbol_changed.has_value()) {
                  local_index_changes.emplace_back(position_idx, *symbol_changed);
               }
            }
         }
      }
   });
   for (const auto& local : index_changes_to_reference) {
      for (const auto& element : local) {
         indexing_differences_to_reference_sequence.emplace_back(element);
      }
   }

   size_t size_of_optimized_positions = computeSize();
   SPDLOG_DEBUG(
      "Size of position indexes before optimization: {}, after: {}, saved: {}",
      size_of_positions,
      size_of_optimized_positions,
      size_of_positions - size_of_optimized_positions
   );

}

template <typename SymbolType>
void silo::SequenceStorePartition<SymbolType>::interpret(
   const std::vector<std::optional<std::string>>& genomes
) {
   fillIndexes(genomes);
   fillNBitmaps(genomes);
   sequence_count += genomes.size();
}

template <typename SymbolType>
size_t silo::SequenceStorePartition<SymbolType>::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      result += position.computeSize();
   }
   return result;
}

template <typename SymbolType>
silo::SequenceStore<SymbolType>::SequenceStore(
   std::vector<typename SymbolType::Symbol> reference_sequence
)
    : reference_sequence(std::move(reference_sequence)) {}

template <typename SymbolType>
silo::SequenceStorePartition<SymbolType>& silo::SequenceStore<SymbolType>::createPartition() {
   return partitions.emplace_back(reference_sequence);
}
template class silo::SequenceStorePartition<silo::Nucleotide>;
template class silo::SequenceStorePartition<silo::AminoAcid>;
template class silo::SequenceStore<silo::Nucleotide>;
template class silo::SequenceStore<silo::AminoAcid>;
