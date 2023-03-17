#include <silo/database.h>
#include <silo/storage/sequence_store.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <external/PerfEvent.hpp>
#include <syncstream>

/// Returns an Roaring-bitmap which has the given residue ambiguous_symbol at the position position,
/// where the residue is interpreted in the _a_pproximate meaning
/// That means a ambiguous_symbol matches all mixed symbols, which can indicate the residue
/// position: 1 indexed position of the genome
roaring::Roaring* silo::SequenceStore::getBitmapFromAmbiguousSymbol(
   size_t position,
   GENOME_SYMBOL ambiguous_symbol
) const {
   static constexpr int COUNT_AMBIGUOUS_SYMBOLS = 8;
   switch (ambiguous_symbol) {
      case A: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, A), getBitmap(position, R), getBitmap(position, W),
            getBitmap(position, M), getBitmap(position, D), getBitmap(position, H),
            getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case C: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, C), getBitmap(position, Y),  // NOLINT(modernize-avoid-c-arrays)
            getBitmap(position, S), getBitmap(position, M), getBitmap(position, B),
            getBitmap(position, H), getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case G: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, G), getBitmap(position, R), getBitmap(position, S),
            getBitmap(position, K), getBitmap(position, D), getBitmap(position, B),
            getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case T: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, T), getBitmap(position, Y), getBitmap(position, W),
            getBitmap(position, K), getBitmap(position, D), getBitmap(position, H),
            getBitmap(position, B)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      default: {
         return new roaring::Roaring(*getBitmap(position, ambiguous_symbol));
      }
   }
}

roaring::Roaring* silo::SequenceStore::getFlippedBitmapFromAmbiguousSymbol(
   size_t position,
   GENOME_SYMBOL ambiguous_symbol
) const {
   const auto* bitmap_to_flip = getBitmap(position, ambiguous_symbol);
   roaring::api::roaring_bitmap_flip(&bitmap_to_flip->roaring, 0, sequence_count);
   static constexpr int COUNT_AMBIGUOUS_SYMBOLS = 8;
   switch (ambiguous_symbol) {
      case A: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,         getBitmap(position, R), getBitmap(position, W),
            getBitmap(position, M), getBitmap(position, D), getBitmap(position, H),
            getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case C: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,         getBitmap(position, Y), getBitmap(position, S),
            getBitmap(position, M), getBitmap(position, B), getBitmap(position, H),
            getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case G: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,         getBitmap(position, R), getBitmap(position, S),
            getBitmap(position, K), getBitmap(position, D), getBitmap(position, B),
            getBitmap(position, V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case T: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,         getBitmap(position, Y), getBitmap(position, W),
            getBitmap(position, K), getBitmap(position, D), getBitmap(position, H),
            getBitmap(position, B)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      default: {
         return new roaring::Roaring(*getBitmap(position, ambiguous_symbol));
      }
   }
}

int silo::SequenceStore::databaseInfo(std::ostream& output_stream) const {
   std::osyncstream(output_stream)
      << "partition sequence count: " << formatNumber(this->sequence_count) << std::endl;
   std::osyncstream(output_stream)
      << "partition index size: " << formatNumber(computeSize()) << std::endl;

   size_t size = 0;
   for (const auto& bitmap : nucleotide_symbol_n_bitmaps) {
      size += bitmap.getSizeInBytes(false);
   }
   std::osyncstream(output_stream)
      << "partition N_bitmap per sequence, total size: " << formatNumber(size) << std::endl;
   return 0;
}

// TODO(someone): reduce cognitive complexity
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void silo::SequenceStore::interpret(const std::vector<std::string>& genomes) {
   const uint32_t cur_sequence_count = sequence_count;
   sequence_count += genomes.size();
   nucleotide_symbol_n_bitmaps.resize(cur_sequence_count + genomes.size());
   {
      static constexpr int COUNT_SYMBOLS_PER_PROCESSOR = 64;
      tbb::blocked_range<unsigned> const range(
         0, GENOME_LENGTH, GENOME_LENGTH / COUNT_SYMBOLS_PER_PROCESSOR
      );
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<std::vector<unsigned>> ids_per_symbol(SYMBOL_COUNT);
         for (unsigned col = local.begin(); col != local.end(); ++col) {
            for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
               char const character = genomes[index2][col];
               GENOME_SYMBOL const symbol = toNucleotideSymbol(character);
               if (symbol != GENOME_SYMBOL::N) {
                  ids_per_symbol[symbol].push_back(cur_sequence_count + index2);
               }
            }
            for (unsigned symbol = 0; symbol != SYMBOL_COUNT; ++symbol) {
               if (!ids_per_symbol[symbol].empty()) {
                  this->positions[col].bitmaps[symbol].addMany(
                     ids_per_symbol[symbol].size(), ids_per_symbol[symbol].data()
                  );
                  ids_per_symbol[symbol].clear();
               }
            }
         }
      });
   }
   {
      tbb::blocked_range<unsigned> const range(0, genomes.size());
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<unsigned> positions_with_nucleotide_symbol_n(SYMBOL_COUNT);
         for (unsigned genome = local.begin(); genome != local.end(); ++genome) {
            for (unsigned pos = 0, limit2 = GENOME_LENGTH; pos != limit2; ++pos) {
               char const character = genomes[genome][pos];
               GENOME_SYMBOL const symbol = toNucleotideSymbol(character);
               if (symbol == GENOME_SYMBOL::N) {
                  positions_with_nucleotide_symbol_n.push_back(pos);
               }
            }
            if (!positions_with_nucleotide_symbol_n.empty()) {
               this->nucleotide_symbol_n_bitmaps[cur_sequence_count + genome].addMany(
                  positions_with_nucleotide_symbol_n.size(),
                  positions_with_nucleotide_symbol_n.data()
               );
               this->nucleotide_symbol_n_bitmaps[cur_sequence_count + genome].runOptimize();
               positions_with_nucleotide_symbol_n.clear();
            }
         }
      });
   }
}

static constexpr int BINARY_CONTAINER_SIZE_OF_BITMAPS = 16;
void silo::SequenceStore::indexAllNucleotideSymbolsN() {
   std::vector<std::vector<std::vector<uint32_t>>> ids_per_position_per_upper(
      (sequence_count >> BINARY_CONTAINER_SIZE_OF_BITMAPS) + 1
   );
   tbb::blocked_range<uint32_t> const range(
      0, (sequence_count >> BINARY_CONTAINER_SIZE_OF_BITMAPS) + 1
   );
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      auto& ids_per_position = ids_per_position_per_upper[local];
      ids_per_position.resize(GENOME_LENGTH);

      uint32_t const genome_upper = local << BINARY_CONTAINER_SIZE_OF_BITMAPS;
      uint32_t const limit = genome_upper == (sequence_count & 0xFFFF0000)
                                ? sequence_count - genome_upper
                                : 1U << BINARY_CONTAINER_SIZE_OF_BITMAPS;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t const position : nucleotide_symbol_n_bitmaps[genome]) {
            ids_per_position[position].push_back(genome);
         }
      }
   });

   for (uint32_t position = 0; position < GENOME_LENGTH; ++position) {
      for (uint32_t upper = 0; upper < (sequence_count >> BINARY_CONTAINER_SIZE_OF_BITMAPS) + 1;
           ++upper) {
         auto& ids = ids_per_position_per_upper[upper][position];
         positions[position].bitmaps[GENOME_SYMBOL::N].addMany(ids.size(), ids.data());
      }
      positions[position].nucleotide_symbol_n_indexed = true;
   }
}

void silo::SequenceStore::naiveIndexAllNucleotideSymbolN() {
   tbb::enumerable_thread_specific<std::vector<std::vector<uint32_t>>> ids_per_position;
   tbb::blocked_range<uint32_t> const range(
      0, (sequence_count >> BINARY_CONTAINER_SIZE_OF_BITMAPS) + 1
   );
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      ids_per_position.local().resize(GENOME_LENGTH);

      uint32_t const genome_upper = local << BINARY_CONTAINER_SIZE_OF_BITMAPS;
      uint32_t const limit = genome_upper == (sequence_count & 0xFFFF0000)
                                ? sequence_count - genome_upper
                                : 1U << BINARY_CONTAINER_SIZE_OF_BITMAPS;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t const position : nucleotide_symbol_n_bitmaps[genome]) {
            ids_per_position.local()[position].push_back(genome);
         }
      }
   });

   for (auto& ids_at_position : ids_per_position) {
      for (uint32_t position = 0; position < GENOME_LENGTH; ++position) {
         auto& ids = ids_at_position[position];
         positions[position].bitmaps[GENOME_SYMBOL::N].addMany(ids.size(), ids.data());
      }
   }

   for (uint32_t position = 0; position < GENOME_LENGTH; ++position) {
      positions[position].nucleotide_symbol_n_indexed = true;
   }
}

/// position: 1 indexed position of the genome
const roaring::Roaring* silo::SequenceStore::getBitmap(size_t position, GENOME_SYMBOL symbol)
   const {
   return &positions[position - 1].bitmaps[symbol];
}
silo::SequenceStore::SequenceStore() = default;
size_t silo::SequenceStore::computeSize() const {
   size_t result = 0;
   for (const auto& position : positions) {
      for (const auto& bitmap : position.bitmaps) {
         result += bitmap.getSizeInBytes(false);
      }
   }
   return result;
}

[[maybe_unused]] unsigned silo::runOptimize(SequenceStore& sequence_store) {
   std::atomic<unsigned> count_true = 0;
   tbb::blocked_range<Position*> const range(
      std::begin(sequence_store.positions), std::end(sequence_store.positions)
   );
   tbb::parallel_for(range, [&](const decltype(range) local) {
      for (Position& position : local) {
         for (auto& bitmap : position.bitmaps) {
            if (bitmap.runOptimize()) {
               ++count_true;
            }
         }
      }
   });
   return count_true;
}

[[maybe_unused]] unsigned silo::shrinkToFit(SequenceStore& sequence_store) {
   std::atomic<size_t> saved = 0;
   tbb::blocked_range<Position*> const range(
      std::begin(sequence_store.positions), std::end(sequence_store.positions)
   );
   tbb::parallel_for(range, [&](const decltype(range) local) {
      size_t local_saved = 0;
      for (Position& position : local) {
         for (auto& bitmap : position.bitmaps) {
            local_saved += bitmap.shrinkToFit();
         }
      }
      saved += local_saved;
   });
   return saved;
}
