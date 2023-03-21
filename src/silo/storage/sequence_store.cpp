#include "silo/storage/sequence_store.h"

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <atomic>
#include <roaring/roaring.hh>
#include <string>
#include <vector>

#include "silo/common/format_number.h"
#include "silo/common/genome_symbols.h"

[[maybe_unused]] auto fmt::formatter<silo::SequenceStoreInfo>::format(
   silo::SequenceStoreInfo sequence_store_info,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
      ctx.out(), "SequenceStoreInfo[sequence count: {}, size: {}, N bitmaps size: {}]",
      sequence_store_info.sequence_count, sequence_store_info.size,
      silo::formatNumber(sequence_store_info.n_bitmaps_size)
   );
}

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
      case GENOME_SYMBOL::A: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, GENOME_SYMBOL::A), getBitmap(position, GENOME_SYMBOL::R),
            getBitmap(position, GENOME_SYMBOL::W), getBitmap(position, GENOME_SYMBOL::M),
            getBitmap(position, GENOME_SYMBOL::D), getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::C: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, GENOME_SYMBOL::C), getBitmap(position, GENOME_SYMBOL::Y),
            getBitmap(position, GENOME_SYMBOL::S), getBitmap(position, GENOME_SYMBOL::M),
            getBitmap(position, GENOME_SYMBOL::B), getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::G: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, GENOME_SYMBOL::G), getBitmap(position, GENOME_SYMBOL::R),
            getBitmap(position, GENOME_SYMBOL::S), getBitmap(position, GENOME_SYMBOL::K),
            getBitmap(position, GENOME_SYMBOL::D), getBitmap(position, GENOME_SYMBOL::B),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::T: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            getBitmap(position, GENOME_SYMBOL::T), getBitmap(position, GENOME_SYMBOL::Y),
            getBitmap(position, GENOME_SYMBOL::W), getBitmap(position, GENOME_SYMBOL::K),
            getBitmap(position, GENOME_SYMBOL::D), getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::B)};
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
      case GENOME_SYMBOL::A: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,
            getBitmap(position, GENOME_SYMBOL::R),
            getBitmap(position, GENOME_SYMBOL::W),
            getBitmap(position, GENOME_SYMBOL::M),
            getBitmap(position, GENOME_SYMBOL::D),
            getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::C: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,
            getBitmap(position, GENOME_SYMBOL::Y),
            getBitmap(position, GENOME_SYMBOL::S),
            getBitmap(position, GENOME_SYMBOL::M),
            getBitmap(position, GENOME_SYMBOL::B),
            getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::G: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,
            getBitmap(position, GENOME_SYMBOL::R),
            getBitmap(position, GENOME_SYMBOL::S),
            getBitmap(position, GENOME_SYMBOL::K),
            getBitmap(position, GENOME_SYMBOL::D),
            getBitmap(position, GENOME_SYMBOL::B),
            getBitmap(position, GENOME_SYMBOL::V)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      case GENOME_SYMBOL::T: {
         // NOLINTNEXTLINE(modernize-avoid-c-arrays)
         const roaring::Roaring* tmp[COUNT_AMBIGUOUS_SYMBOLS] = {
            bitmap_to_flip,
            getBitmap(position, GENOME_SYMBOL::Y),
            getBitmap(position, GENOME_SYMBOL::W),
            getBitmap(position, GENOME_SYMBOL::K),
            getBitmap(position, GENOME_SYMBOL::D),
            getBitmap(position, GENOME_SYMBOL::H),
            getBitmap(position, GENOME_SYMBOL::B)};
         auto* result =
            new roaring::Roaring(roaring::Roaring::fastunion(COUNT_AMBIGUOUS_SYMBOLS, tmp));
         return result;
      }
      default: {
         return new roaring::Roaring(*getBitmap(position, ambiguous_symbol));
      }
   }
}

silo::SequenceStoreInfo silo::SequenceStore::getInfo() const {
   size_t n_bitmaps_size = 0;
   for (const auto& bitmap : nucleotide_symbol_n_bitmaps) {
      n_bitmaps_size += bitmap.getSizeInBytes(false);
   }
   return SequenceStoreInfo{this->sequence_count, computeSize(), n_bitmaps_size};
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
                  ids_per_symbol[static_cast<unsigned>(symbol)].push_back(
                     cur_sequence_count + index2
                  );
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
         positions[position].bitmaps[static_cast<unsigned>(GENOME_SYMBOL::N)].addMany(
            ids.size(), ids.data()
         );
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
         positions[position].bitmaps[static_cast<unsigned>(GENOME_SYMBOL::N)].addMany(
            ids.size(), ids.data()
         );
      }
   }

   for (uint32_t position = 0; position < GENOME_LENGTH; ++position) {
      positions[position].nucleotide_symbol_n_indexed = true;
   }
}

/// position: 1 indexed position of the genome
const roaring::Roaring* silo::SequenceStore::getBitmap(size_t position, GENOME_SYMBOL symbol)
   const {
   return &positions[position - 1].bitmaps[static_cast<unsigned>(symbol)];
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
