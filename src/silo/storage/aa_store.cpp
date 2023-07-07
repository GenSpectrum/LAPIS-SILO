#include "silo/storage/aa_store.h"

#include <atomic>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/zstdfasta_reader.h"

size_t silo::AAStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr size_t BUFFER_SIZE = 1024;

   size_t read_sequences_count = 0;

   std::vector<std::string> sequence_buffer;

   std::string key;
   std::string sequence;
   while (input_file.next(key, sequence)) {
      sequence_buffer.push_back(std::move(sequence));
      if (sequence_buffer.size() >= BUFFER_SIZE) {
         interpret(sequence_buffer);
         sequence_buffer.clear();
      }

      ++read_sequences_count;
   }
   interpret(sequence_buffer);

   return read_sequences_count;
}

const roaring::Roaring* silo::AAStorePartition::getBitmap(size_t position, AA_SYMBOL symbol) const {
   return &positions[position].bitmaps[static_cast<uint32_t>(symbol)];
}

void silo::AAStorePartition::fillIndexes(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   const tbb::blocked_range<size_t> positions_range(
      0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR
   );
   tbb::parallel_for(positions_range, [&](const decltype(positions_range)& local) {
      std::vector<std::vector<uint32_t>> ids_per_symbol_for_current_position(AA_SYMBOL_COUNT);
      for (size_t position = local.begin(); position != local.end(); ++position) {
         const size_t number_of_sequences = sequences.size();
         for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
            const char character = sequences[sequence_id][position];
            const AA_SYMBOL symbol = toAASymbol(character).value_or(AA_SYMBOL::X);
            if (symbol != AA_SYMBOL::X) {
               ids_per_symbol_for_current_position[static_cast<uint32_t>(symbol)].push_back(
                  sequence_count + sequence_id
               );
            }
         }
         for (const auto& symbol : AA_SYMBOLS) {
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

void silo::AAStorePartition::fillXBitmaps(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();

   aa_symbol_x_bitmaps.resize(sequence_count + sequences.size());

   const tbb::blocked_range<size_t> range(0, sequences.size());
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      // For every symbol, calculate all sequence IDs that have that symbol at that position
      std::vector<uint32_t> positions_with_aa_symbol_x;
      for (size_t sequence_id = local.begin(); sequence_id != local.end(); ++sequence_id) {
         for (size_t position = 0; position < genome_length; ++position) {
            const char character = sequences[sequence_id][position];
            const AA_SYMBOL symbol = toAASymbol(character).value_or(AA_SYMBOL::X);
            if (symbol == AA_SYMBOL::X) {
               positions_with_aa_symbol_x.push_back(position);
            }
         }
         if (!positions_with_aa_symbol_x.empty()) {
            aa_symbol_x_bitmaps[sequence_count + sequence_id].addMany(
               positions_with_aa_symbol_x.size(), positions_with_aa_symbol_x.data()
            );
            aa_symbol_x_bitmaps[sequence_count + sequence_id].runOptimize();
            positions_with_aa_symbol_x.clear();
         }
      }
   });
}

void silo::AAStorePartition::interpret(const std::vector<std::string>& sequences) {
   fillIndexes(sequences);
   fillXBitmaps(sequences);
   sequence_count += sequences.size();
}

silo::AAStorePartition::AAStorePartition(const std::string& reference_sequence)
    : reference_sequence(reference_sequence),
      positions(reference_sequence.length()) {}

size_t silo::AAStorePartition::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const auto& bitmap : position.bitmaps) {
         result += bitmap.getSizeInBytes(false);
      }
   }
   return result;
}

size_t silo::AAStorePartition::runOptimize() {
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

size_t silo::AAStorePartition::shrinkToFit() {
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

silo::AAStore::AAStore(std::string reference_sequence)
    : reference_sequence(std::move(reference_sequence)) {}

silo::AAStorePartition& silo::AAStore::createPartition() {
   return partitions.emplace_back(reference_sequence);
}
