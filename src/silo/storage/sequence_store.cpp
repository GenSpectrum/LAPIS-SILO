#include "silo/storage/sequence_store.h"

#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

template <typename SymbolType>
silo::Position<SymbolType>::Position(typename SymbolType::Symbol symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

template <typename SymbolType>
silo::Position<SymbolType>::Position(std::optional<typename SymbolType::Symbol> symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

template <typename SymbolType>
std::optional<typename SymbolType::Symbol> silo::Position<SymbolType>::flipMostNumerousBitmap(
   uint32_t sequence_count
) {
   std::optional<typename SymbolType::Symbol> flipped_bitmap_before =
      symbol_whose_bitmap_is_flipped;
   std::optional<typename SymbolType::Symbol> max_symbol = std::nullopt;
   uint32_t max_count = 0;

   for (const auto& symbol : SymbolType::SYMBOLS) {
      roaring::Roaring& bitmap = bitmaps[symbol];
      bitmap.runOptimize();
      bitmap.shrinkToFit();
      const uint32_t count = flipped_bitmap_before == symbol ? sequence_count - bitmap.cardinality()
                                                             : bitmap.cardinality();
      if (count > max_count) {
         max_symbol = symbol;
         max_count = count;
      }
   }
   if (max_symbol != flipped_bitmap_before) {
      if (flipped_bitmap_before.has_value()) {
         bitmaps[*flipped_bitmap_before].flip(0, sequence_count);
         bitmaps[*flipped_bitmap_before].runOptimize();
         bitmaps[*flipped_bitmap_before].shrinkToFit();
      }
      if (max_symbol.has_value()) {
         bitmaps[*max_symbol].flip(0, sequence_count);
         bitmaps[*max_symbol].runOptimize();
         bitmaps[*max_symbol].shrinkToFit();
      }
      symbol_whose_bitmap_is_flipped = max_symbol;
      return symbol_whose_bitmap_is_flipped;
   }
   return std::nullopt;
}

template <typename SymbolType>
silo::SequenceStorePartition<SymbolType>::SequenceStorePartition(
   const std::vector<typename SymbolType::Symbol>& reference_sequence
)
    : reference_sequence(reference_sequence) {
   positions.reserve(reference_sequence.size());
   for (const auto symbol : reference_sequence) {
      positions.emplace_back(symbol);
   }
}

template <typename Symbol>
size_t silo::SequenceStorePartition<Symbol>::fill(ZstdFastaTableReader& input) {
   static constexpr size_t BUFFER_SIZE = 1024;

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
   SPDLOG_DEBUG("Sequence store partition info after filling it: {}", getInfo());

   return read_sequences_count;
}

[[maybe_unused]] auto fmt::formatter<silo::SequenceStoreInfo>::format(
   silo::SequenceStoreInfo sequence_store_info,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
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
   size_t position,
   typename SymbolType::Symbol symbol
) const {
   return &positions[position].bitmaps.at(symbol);
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
         for (size_t position = local.begin(); position != local.end(); ++position) {
            const size_t number_of_sequences = genomes.size();
            for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
               const auto& genome = genomes[sequence_id];
               if (!genome.has_value()) {
                  continue;
               }
               char const character = genome.value()[position];
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
            for (const auto& symbol : SymbolType::SYMBOLS) {
               if (!ids_per_symbol_for_current_position.at(symbol).empty()) {
                  positions[position].bitmaps[symbol].addMany(
                     ids_per_symbol_for_current_position.at(symbol).size(),
                     ids_per_symbol_for_current_position.at(symbol).data()
                  );
                  ids_per_symbol_for_current_position[symbol].clear();
               }
               if (symbol == positions[position].symbol_whose_bitmap_is_flipped) {
                  positions[position].bitmaps[symbol].flip(
                     sequence_count, sequence_count + number_of_sequences
                  );
               }
            }
         }
      }
   );
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

         for (size_t position = 0; position < genome_length; ++position) {
            char const character = genome[position];
            const auto symbol = SymbolType::charToSymbol(character);
            if (symbol == SymbolType::SYMBOL_MISSING) {
               positions_with_symbol_missing.push_back(position);
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
      for (const auto symbol : SymbolType::SYMBOLS) {
         result += position.bitmaps.at(symbol).getSizeInBytes(false);
      }
   }
   return result;
}

template <typename SymbolType>
silo::SequenceStore<SymbolType>::SequenceStore(
   std::vector<typename SymbolType::Symbol> reference_sequence
)
    : reference_sequence(std::move(reference_sequence)) {}

template <typename Symbol>
silo::SequenceStorePartition<Symbol>& silo::SequenceStore<Symbol>::createPartition() {
   return partitions.emplace_back(reference_sequence);
}

template class silo::Position<silo::Nucleotide>;
template class silo::Position<silo::AminoAcid>;
template class silo::SequenceStorePartition<silo::Nucleotide>;
template class silo::SequenceStorePartition<silo::AminoAcid>;
template class silo::SequenceStore<silo::Nucleotide>;
template class silo::SequenceStore<silo::AminoAcid>;
