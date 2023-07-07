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

size_t silo::SequenceStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr size_t BUFFER_SIZE = 1024;

   size_t read_sequences_count = 0;

   std::vector<std::string> genome_buffer;

   std::string key;
   std::string genome;
   while (input_file.next(key, genome)) {
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

silo::SequenceStoreInfo silo::SequenceStorePartition::getInfo() const {
   size_t n_bitmaps_size = 0;
   for (const auto& bitmap : nucleotide_symbol_n_bitmaps) {
      n_bitmaps_size += bitmap.getSizeInBytes(false);
   }
   return SequenceStoreInfo{this->sequence_count, computeSize(), n_bitmaps_size};
}

const roaring::Roaring* silo::SequenceStorePartition::getBitmap(
   size_t position,
   NUCLEOTIDE_SYMBOL symbol
) const {
   return &positions[position].bitmaps[static_cast<uint32_t>(symbol)];
}

void silo::SequenceStorePartition::fillIndexes(const std::vector<std::string>& genomes) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   const tbb::blocked_range<size_t> range(
      0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR
   );
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      std::vector<std::vector<uint32_t>> ids_per_symbol_for_current_position(NUC_SYMBOL_COUNT);
      for (size_t position = local.begin(); position != local.end(); ++position) {
         const size_t number_of_genomes = genomes.size();
         for (size_t sequence_id = 0; sequence_id < number_of_genomes; ++sequence_id) {
            char const character = genomes[sequence_id][position];
            const NUCLEOTIDE_SYMBOL symbol =
               toNucleotideSymbol(character).value_or(NUCLEOTIDE_SYMBOL::N);
            if (symbol != NUCLEOTIDE_SYMBOL::N) {
               ids_per_symbol_for_current_position[static_cast<uint32_t>(symbol)].push_back(
                  sequence_count + sequence_id
               );
            }
         }
         for (const auto& symbol : NUC_SYMBOLS) {
            const auto symbol_index = static_cast<uint32_t>(symbol);
            if (!ids_per_symbol_for_current_position[symbol_index].empty()) {
               this->positions[position].bitmaps[symbol_index].addMany(
                  ids_per_symbol_for_current_position[symbol_index].size(),
                  ids_per_symbol_for_current_position[symbol_index].data()
               );
               ids_per_symbol_for_current_position[symbol_index].clear();
            }
         }
      }
   });
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
            const NUCLEOTIDE_SYMBOL symbol =
               toNucleotideSymbol(character).value_or(NUCLEOTIDE_SYMBOL::N);
            if (symbol == NUCLEOTIDE_SYMBOL::N) {
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

silo::SequenceStorePartition::SequenceStorePartition(const std::string& reference_genome)
    : reference_genome(reference_genome),
      positions(reference_genome.length()) {}

size_t silo::SequenceStorePartition::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const auto& bitmap : position.bitmaps) {
         result += bitmap.getSizeInBytes(false);
      }
   }
   return result;
}

size_t silo::SequenceStorePartition::runOptimize() {
   std::atomic<size_t> count_true = 0;
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (auto& bitmap : positions[position].bitmaps) {
            if (bitmap.runOptimize()) {
               ++count_true;
            }
         }
      }
   });
   return count_true;
}

size_t silo::SequenceStorePartition::shrinkToFit() {
   std::atomic<size_t> saved = 0;
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      size_t local_saved = 0;
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (auto& bitmap : positions[position].bitmaps) {
            local_saved += bitmap.shrinkToFit();
         }
      }
      saved += local_saved;
   });
   return saved;
}

silo::SequenceStore::SequenceStore(std::string reference_genome)
    : reference_genome(std::move(reference_genome)) {}

silo::SequenceStorePartition& silo::SequenceStore::createPartition() {
   return partitions.emplace_back(reference_genome);
}
