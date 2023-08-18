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
#include "silo/preprocessing/preprocessing_exception.h"

silo::AAPosition::AAPosition(AA_SYMBOL symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

silo::AAPosition::AAPosition(std::optional<AA_SYMBOL> symbol) {
   symbol_whose_bitmap_is_flipped = symbol;
}

std::optional<silo::AA_SYMBOL> silo::AAPosition::flipMostNumerousBitmap(uint32_t sequence_count) {
   std::optional<AA_SYMBOL> previous_flipped_bitmap_symbol = symbol_whose_bitmap_is_flipped;
   std::optional<AA_SYMBOL> new_flipped_bitmap_symbol = std::nullopt;
   uint32_t max_count = 0;

   for (const auto& symbol : Util<AA_SYMBOL>::symbols) {
      roaring::Roaring bitmap = bitmaps.at(symbol);
      bitmap.runOptimize();
      bitmap.shrinkToFit();
      const uint32_t count = previous_flipped_bitmap_symbol == symbol
                                ? sequence_count - bitmap.cardinality()
                                : bitmap.cardinality();
      if (count > max_count) {
         new_flipped_bitmap_symbol = symbol;
         max_count = count;
      }
   }
   if (new_flipped_bitmap_symbol != previous_flipped_bitmap_symbol) {
      if (previous_flipped_bitmap_symbol.has_value()) {
         bitmaps[*previous_flipped_bitmap_symbol].flip(0, sequence_count);
         bitmaps[*previous_flipped_bitmap_symbol].runOptimize();
         bitmaps[*previous_flipped_bitmap_symbol].shrinkToFit();
      }
      if (new_flipped_bitmap_symbol.has_value()) {
         bitmaps[*new_flipped_bitmap_symbol].flip(0, sequence_count);
         bitmaps[*new_flipped_bitmap_symbol].runOptimize();
         bitmaps[*new_flipped_bitmap_symbol].shrinkToFit();
      }
      symbol_whose_bitmap_is_flipped = new_flipped_bitmap_symbol;
      return symbol_whose_bitmap_is_flipped;
   }
   return std::nullopt;
}

silo::AAStorePartition::AAStorePartition(const std::vector<AA_SYMBOL>& reference_sequence)
    : reference_sequence(reference_sequence) {
   for (const AA_SYMBOL symbol : reference_sequence) {
      positions.emplace_back(symbol);
   }
}

size_t silo::AAStorePartition::fill(silo::ZstdFastaReader& input_file) {
   static constexpr size_t BUFFER_SIZE = 1024;

   size_t read_sequences_count = 0;

   std::vector<std::string> sequence_buffer;

   std::optional<std::string> key;
   std::string sequence;
   while (true) {
      key = input_file.next(sequence);
      if (!key) {
         break;
      }
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
   return &positions[position].bitmaps.at(symbol);
}

void silo::AAStorePartition::fillIndexes(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();
   static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, genome_length, genome_length / COUNT_SYMBOLS_PER_PROCESSOR),
      [&](const auto& local) {
         AASymbolMap<std::vector<uint32_t>> ids_per_symbol_for_current_position;
         for (size_t position = local.begin(); position != local.end(); ++position) {
            const size_t number_of_sequences = sequences.size();
            for (size_t sequence_id = 0; sequence_id < number_of_sequences; ++sequence_id) {
               const char character = sequences[sequence_id][position];
               const auto symbol = Util<AA_SYMBOL>::charToSymbol(character);
               if (!symbol.has_value()) {
                  throw PreprocessingException(
                     "Found invalid symbol in Amino Acid sequence: " + std::to_string(character) +
                     "\nFull sequence: " + sequences[sequence_id]
                  );
               }
               if (symbol != AA_SYMBOL::X) {
                  ids_per_symbol_for_current_position[*symbol].push_back(
                     sequence_count + sequence_id
                  );
               }
            }
            for (const AA_SYMBOL symbol : Util<AA_SYMBOL>::symbols) {
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

void silo::AAStorePartition::fillXBitmaps(const std::vector<std::string>& sequences) {
   const size_t genome_length = positions.size();

   aa_symbol_x_bitmaps.resize(sequence_count + sequences.size());

   tbb::parallel_for(tbb::blocked_range<size_t>(0, sequences.size()), [&](const auto& local) {
      std::vector<uint32_t> positions_with_aa_symbol_x;
      for (size_t sequence_id = local.begin(); sequence_id != local.end(); ++sequence_id) {
         for (size_t position = 0; position < genome_length; ++position) {
            const char character = sequences[sequence_id][position];
            // No need to check the cast because we call fillIndexes first
            const auto symbol = Util<AA_SYMBOL>::charToSymbol(character);
            if (symbol == AA_SYMBOL::X) {
               positions_with_aa_symbol_x.push_back(position);
            }
         }
         if (!positions_with_aa_symbol_x.empty()) {
            aa_symbol_x_bitmaps[sequence_count + sequence_id].addMany(
               positions_with_aa_symbol_x.size(), positions_with_aa_symbol_x.data()
            );
            aa_symbol_x_bitmaps[sequence_count + sequence_id].runOptimize();
            aa_symbol_x_bitmaps[sequence_count + sequence_id].shrinkToFit();
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

silo::AAStore::AAStore(std::vector<AA_SYMBOL> reference_sequence)
    : reference_sequence(std::move(reference_sequence)) {}

silo::AAStorePartition& silo::AAStore::createPartition() {
   return partitions.emplace_back(reference_sequence);
}
