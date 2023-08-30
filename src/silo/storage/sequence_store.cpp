#include "silo/storage/sequence_store.h"

#include <atomic>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"

silo::NucPosition::NucPosition(Nucleotide::Symbol symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

silo::NucPosition::NucPosition(std::optional<Nucleotide::Symbol> symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

std::optional<silo::Nucleotide::Symbol> silo::NucPosition::flipMostNumerousBitmap(
   uint32_t sequence_count
) {
   std::optional<Nucleotide::Symbol> flipped_bitmap_before = symbol_whose_bitmap_is_flipped;
   std::optional<Nucleotide::Symbol> max_symbol = std::nullopt;
   uint32_t max_count = 0;

   for (const auto& symbol : Nucleotide::SYMBOLS) {
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

silo::SequenceStorePartition::SequenceStorePartition(
   const std::vector<Nucleotide::Symbol>& reference_genome
)
    : reference_genome(reference_genome) {}

size_t silo::SequenceStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr size_t BUFFER_SIZE = 1024;

   for (const Nucleotide::Symbol symbol : reference_genome) {
      positions.emplace_back(symbol);
   }

   size_t read_sequences_count = 0;

   std::vector<std::string> genome_buffer;

   std::optional<std::string> key;
   std::string genome;
   while (true) {
      key = input_file.next(genome);
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
   SPDLOG_DEBUG("{}", getInfo());

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

silo::SequenceStoreInfo silo::SequenceStorePartition::getInfo() const {
   size_t n_bitmaps_size = 0;
   for (const auto& bitmap : nucleotide_symbol_n_bitmaps) {
      n_bitmaps_size += bitmap.getSizeInBytes(false);
   }
   return SequenceStoreInfo{this->sequence_count, computeSize(), n_bitmaps_size};
}

const roaring::Roaring* silo::SequenceStorePartition::getBitmap(
   size_t position,
   Nucleotide::Symbol symbol
) const {
   return &positions[position].bitmaps.at(symbol);
}

void silo::SequenceStorePartition::fillIndexes(const std::vector<std::string>& genomes) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR),
      [&](const auto& local) {
         SymbolMap<Nucleotide, std::vector<uint32_t>> ids_per_symbol_for_current_position;
         for (size_t position = local.begin(); position != local.end(); ++position) {
            const size_t number_of_sequences = genomes.size();
            for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
               char const character = genomes[sequence_id][position];
               const auto symbol = Nucleotide::charToSymbol(character);
               if (!symbol.has_value()) {
                  throw PreprocessingException(
                     "Illegal character " + std::to_string(character) + " contained in sequence."
                  );
               }
               if (symbol != Nucleotide::Symbol::N) {
                  ids_per_symbol_for_current_position[*symbol].push_back(
                     sequence_count + sequence_id
                  );
               }
            }
            for (const auto& symbol : Nucleotide::SYMBOLS) {
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

void silo::SequenceStorePartition::fillNBitmaps(const std::vector<std::string>& genomes) {
   const size_t genome_length = positions.size();

   nucleotide_symbol_n_bitmaps.resize(sequence_count + genomes.size());

   const tbb::blocked_range<size_t> range(0, genomes.size());
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      // For every symbol, calculate all sequence IDs that have that symbol at that position
      std::vector<uint32_t> positions_with_nucleotide_symbol_n;
      for (size_t genome = local.begin(); genome != local.end(); ++genome) {
         for (size_t position = 0; position < genome_length; ++position) {
            char const character = genomes[genome][position];
            const auto symbol = Nucleotide::charToSymbol(character);
            if (symbol == Nucleotide::Symbol::N) {
               positions_with_nucleotide_symbol_n.push_back(position);
            }
         }
         if (!positions_with_nucleotide_symbol_n.empty()) {
            nucleotide_symbol_n_bitmaps[sequence_count + genome].addMany(
               positions_with_nucleotide_symbol_n.size(), positions_with_nucleotide_symbol_n.data()
            );
            nucleotide_symbol_n_bitmaps[sequence_count + genome].runOptimize();
            positions_with_nucleotide_symbol_n.clear();
         }
      }
   });
}

void silo::SequenceStorePartition::interpret(const std::vector<std::string>& genomes) {
   fillIndexes(genomes);
   fillNBitmaps(genomes);
   sequence_count += genomes.size();
}

size_t silo::SequenceStorePartition::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const Nucleotide::Symbol symbol : Nucleotide::SYMBOLS) {
         result += position.bitmaps.at(symbol).getSizeInBytes(false);
      }
   }
   return result;
}

silo::SequenceStore::SequenceStore(std::vector<Nucleotide::Symbol> reference_genome)
    : reference_genome(std::move(reference_genome)) {}

silo::SequenceStorePartition& silo::SequenceStore::createPartition() {
   return partitions.emplace_back(reference_genome);
}
