#include "silo/storage/sequence_store.h"

#include <algorithm>
#include <atomic>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_for_each.h>
#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>

#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/preprocessing/preprocessing_exception.h"

silo::NucPosition::NucPosition(NUCLEOTIDE_SYMBOL symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

silo::NucPosition::NucPosition(std::optional<NUCLEOTIDE_SYMBOL> symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

std::optional<silo::NUCLEOTIDE_SYMBOL> silo::NucPosition::flipMostNumerousBitmap(
   uint32_t sequence_count
) {
   std::optional<NUCLEOTIDE_SYMBOL> flipped_bitmap_before = symbol_whose_bitmap_is_flipped;
   std::optional<NUCLEOTIDE_SYMBOL> max_symbol = std::nullopt;
   uint32_t max_count = 0;

   for (const auto& symbol : NUC_SYMBOLS) {
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
   const std::vector<NUCLEOTIDE_SYMBOL>& reference_genome
)
    : reference_genome(reference_genome) {
   for (const NUCLEOTIDE_SYMBOL symbol : reference_genome) {
      positions.emplace_back(symbol);
   }
}

size_t silo::SequenceStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr size_t BUFFER_SIZE = 1024;

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
   fillMutationFilter();
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
   NUCLEOTIDE_SYMBOL symbol
) const {
   return &positions[position].bitmaps.at(symbol);
}

void silo::SequenceStorePartition::fillIndexes(const std::vector<std::string>& genomes) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR),
      [&](const auto& local) {
         NucleotideSymbolMap<std::vector<uint32_t>> ids_per_symbol_for_current_position;
         for (size_t position = local.begin(); position != local.end(); ++position) {
            const size_t number_of_sequences = genomes.size();
            for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
               char const character = genomes[sequence_id][position];
               const auto symbol = charToNucleotideSymbol(character);
               if (!symbol.has_value()) {
                  throw PreprocessingException(
                     "Illegal character " + std::to_string(character) + " contained in sequence."
                  );
               }
               if (symbol != NUCLEOTIDE_SYMBOL::N) {
                  ids_per_symbol_for_current_position[*symbol].push_back(
                     sequence_count + sequence_id
                  );
               }
            }
            for (const auto& symbol : NUC_SYMBOLS) {
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
            const auto symbol = charToNucleotideSymbol(character);
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

void silo::SequenceStorePartition::fillMutationFilter() {
   const auto start_length = 8;
   std::vector<std::pair<uint32_t, uint32_t>> slice_lengths_mutation_counts;

   for (uint32_t slice_length = start_length; slice_length <= reference_genome.size();
        slice_length *= 2) {
      for (uint32_t div = start_length; div >= 1; div /= 2) {
         const auto mutation_count = slice_length / div;
         slice_lengths_mutation_counts.emplace_back(slice_length, mutation_count);
      }
   }
   auto get_mutated_ids_for_genome_pos = [&](uint32_t genome_pos) {
      std::vector<const roaring::Roaring*> filtered_symbols_bitmap;
      for (const auto nuc_symbol : NUC_SYMBOLS) {
         if (nuc_symbol != reference_genome[genome_pos] && nuc_symbol != NUCLEOTIDE_SYMBOL::N && nuc_symbol != NUCLEOTIDE_SYMBOL::GAP) {
            filtered_symbols_bitmap.push_back(getBitmap(genome_pos, nuc_symbol));
         }
      }
      return std::make_unique<roaring::Roaring>(
         roaring::Roaring::fastunion(filtered_symbols_bitmap.size(), filtered_symbols_bitmap.data())
      );
   };

   auto get_mutated_ids_for_range_and_mut_count =
      [&](
         std::vector<uint32_t>& counts,
         const std::pair<uint32_t, uint32_t> genome_range,
         const uint32_t mutation_count
      ) {
         const auto [genome_start_pos, genome_end_pos] = genome_range;
         if (genome_start_pos > 0) {
            auto mutated_ids = get_mutated_ids_for_genome_pos(genome_start_pos);
            for (auto genome_id : *mutated_ids) {
               --counts[genome_id];
            }
         }
         if (genome_end_pos < reference_genome.size()) {
            auto mutated_ids = get_mutated_ids_for_genome_pos(genome_start_pos);
            for (auto genome_id : *mutated_ids) {
               ++counts[genome_id];
            }
         }
         std::vector<uint32_t> genome_ids;
         for (size_t id = 0; id < sequence_count; ++id) {
            if (counts[id] >= mutation_count) {
               genome_ids.push_back(id);
            }
         }
         return std::make_unique<roaring::Roaring>(genome_ids.size(), genome_ids.data());
      };

   std::mutex mutex;
   tbb::parallel_for_each(
      slice_lengths_mutation_counts.begin(),
      slice_lengths_mutation_counts.end(),
      [&](const auto slice_length_mutation_count) {
         const auto slice_length = slice_length_mutation_count.first;
         const auto overlap_shift = slice_length / 2;
         const auto mutation_count = slice_length_mutation_count.second;
         std::vector<std::unique_ptr<roaring::Roaring>> mutation_filter_bitmaps;
         std::vector<uint32_t> counts(sequence_count, 0);

         for (uint32_t genome_pos = 0; genome_pos < slice_length; ++genome_pos) {
            auto mutated_ids = get_mutated_ids_for_genome_pos(genome_pos);
            for (auto genome_id : *mutated_ids) {
               ++counts[genome_id];
            }
         }

         for (uint32_t genome_start_pos = 0; genome_start_pos < reference_genome.size();
              genome_start_pos += overlap_shift) {
            mutation_filter_bitmaps.push_back(get_mutated_ids_for_range_and_mut_count(
               counts,
               std::make_pair(genome_start_pos, genome_start_pos + slice_length),
               mutation_count
            ));
         }
         const std::unique_lock<std::mutex> lock{mutex};
         mutation_filter.addSliceIdx(
            slice_length, overlap_shift, mutation_count, std::move(mutation_filter_bitmaps)
         );
      }
   );

   mutation_filter.finalize();
}

void silo::SequenceStorePartition::interpret(const std::vector<std::string>& genomes) {
   fillIndexes(genomes);
   fillNBitmaps(genomes);
   sequence_count += genomes.size();
}

size_t silo::SequenceStorePartition::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
         result += position.bitmaps.at(symbol).getSizeInBytes(false);
      }
   }
   result += mutation_filter.computeSize();
   return result;
}

size_t silo::SequenceStorePartition::runOptimize() {
   std::atomic<size_t> count_true = 0;
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
            if (positions[position].bitmaps[symbol].runOptimize()) {
               ++count_true;
            }
         }
      }
   });
   count_true += mutation_filter.runOptimize();
   return count_true;
}

size_t silo::SequenceStorePartition::shrinkToFit() {
   std::atomic<size_t> saved = 0;
   const tbb::blocked_range<size_t> range(0U, positions.size());
   tbb::parallel_for(range, [&](const decltype(range) local) {
      size_t local_saved = 0;
      for (auto position = local.begin(); position != local.end(); ++position) {
         for (const NUCLEOTIDE_SYMBOL symbol : NUC_SYMBOLS) {
            local_saved += positions[position].bitmaps[symbol].shrinkToFit();
         }
      }
      saved += local_saved;
   });
   saved += mutation_filter.shrinkToFit();
   return saved;
}

silo::SequenceStore::SequenceStore(std::vector<NUCLEOTIDE_SYMBOL> reference_genome)
    : reference_genome(std::move(reference_genome)) {}

silo::SequenceStorePartition& silo::SequenceStore::createPartition() {
   return partitions.emplace_back(reference_genome);
}
