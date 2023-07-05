#include "silo/storage/aa_store.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"

unsigned silo::AAStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr unsigned BUFFER_SIZE = 1024;

   unsigned read_sequences_count = 0;

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
   return &positions[position].bitmaps[static_cast<unsigned>(symbol)];
}

void silo::AAStorePartition::fillIndexes(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   tbb::blocked_range<unsigned> const range(
      0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR
   );
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      /// For every symbol, calculate all sequence IDs that have that symbol at that position
      std::vector<std::vector<unsigned>> ids_per_symbol(AA_SYMBOL_COUNT);
      for (unsigned col = local.begin(); col != local.end(); ++col) {
         for (unsigned index2 = 0, limit2 = sequences.size(); index2 != limit2; ++index2) {
            char const character = sequences[index2][col];
            const AA_SYMBOL symbol = toAASymbol(character).value_or(AA_SYMBOL::X);
            if (symbol != AA_SYMBOL::X) {
               ids_per_symbol[static_cast<unsigned>(symbol)].push_back(sequence_count + index2);
            }
         }
         for (const auto& symbol : AA_SYMBOLS) {
            const auto symbol_index = static_cast<unsigned>(symbol);
            if (!ids_per_symbol[symbol_index].empty()) {
               this->positions[col].bitmaps[symbol_index].addMany(
                  ids_per_symbol[symbol_index].size(), ids_per_symbol[symbol_index].data()
               );
               ids_per_symbol[symbol_index].clear();
            }
         }
      }
   });
}

void silo::AAStorePartition::fillXBitmaps(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();

   aa_symbol_x_bitmaps.resize(sequence_count + sequences.size());

   tbb::blocked_range<unsigned> const range(0, sequences.size());
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      /// For every symbol, calculate all sequence IDs that have that symbol at that position
      std::vector<unsigned> positions_with_aa_symbol_x;
      for (unsigned genome = local.begin(); genome != local.end(); ++genome) {
         for (unsigned pos = 0, limit2 = genome_length; pos != limit2; ++pos) {
            char const character = sequences[genome][pos];
            const AA_SYMBOL symbol = toAASymbol(character).value_or(AA_SYMBOL::X);
            if (symbol == AA_SYMBOL::X) {
               positions_with_aa_symbol_x.push_back(pos);
            }
         }
         if (!positions_with_aa_symbol_x.empty()) {
            aa_symbol_x_bitmaps[sequence_count + genome].addMany(
               positions_with_aa_symbol_x.size(), positions_with_aa_symbol_x.data()
            );
            aa_symbol_x_bitmaps[sequence_count + genome].runOptimize();
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
      positions(reference_sequence.length()){};

size_t silo::AAStorePartition::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const auto& bitmap : position.bitmaps) {
         result += bitmap.getSizeInBytes(false);
      }
   }
   return result;
}

unsigned silo::AAStorePartition::runOptimize() {
   std::atomic<unsigned> count_true = 0;
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

unsigned silo::AAStorePartition::shrinkToFit() {
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
