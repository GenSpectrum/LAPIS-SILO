#pragma once

#include <expected>
#include <optional>
#include <vector>

#include "silo/append/append_exception.h"

namespace silo {

class Coverage {
  public:
   uint32_t start = 0;
   uint32_t end = 0;
   std::vector<uint32_t> missing_positions;
};

/// Also includes Deletions, which are "mutations to GAP"
template <typename SymbolType>
class Mutations {
  public:
   std::vector<std::pair<uint32_t, typename SymbolType::Symbol>> mutations;
};

class Insertions {
  public:
   std::vector<std::string> insertions;
};

template <typename SymbolType>
class CoverageAndMutations {
  public:
   Coverage coverage;
   Mutations<SymbolType> mutations;
};

template <typename SymbolType>
std::expected<CoverageAndMutations<SymbolType>, std::string>
extractCoverageAndMutationsFromSequence(
   std::string_view sequence,
   size_t offset,
   std::string_view reference
) {
   Coverage coverage;
   Mutations<SymbolType> mutations;

   // Single pass over the sequence computes both the covered range / missing
   // positions (coverage) and the deviations from the local reference
   // (mutations), mirroring HorizontalCoverageIndex::insertSequenceCoverage and
   // the diff loop that used to live in SequenceColumn::appendValue.
   std::optional<uint32_t> first_non_missing;
   std::optional<uint32_t> last_non_missing;
   for (uint32_t char_in_sequence = 0; char_in_sequence < sequence.size(); ++char_in_sequence) {
      const uint32_t position_idx = char_in_sequence + offset;
      const char character = sequence[char_in_sequence];
      const auto symbol = SymbolType::charToSymbol(character);
      if (!symbol.has_value()) {
         return std::unexpected{fmt::format(
            "illegal character '{}' at position {} in the input sequence", character, position_idx
         )};
      }
      if (symbol == SymbolType::SYMBOL_MISSING) {
         coverage.missing_positions.push_back(position_idx);
         continue;
      }
      if (!first_non_missing.has_value()) {
         first_non_missing = position_idx;
      }
      last_non_missing = position_idx;
      if (character != reference[position_idx]) {
         mutations.mutations.emplace_back(position_idx, symbol.value());
      }
   }

   if (first_non_missing.has_value()) {
      coverage.start = first_non_missing.value();
      coverage.end = last_non_missing.value() + 1;
      // Missing positions outside the covered region carry no coverage
      // information (insertCoverage would trim them anyway), so drop them here to
      // keep the buffered chunk as small as possible.
      std::erase_if(coverage.missing_positions, [&](uint32_t position_idx) {
         return position_idx < coverage.start || position_idx >= coverage.end;
      });
   } else {
      // Fully missing sequence: no covered region and nothing to record.
      coverage.missing_positions.clear();
   }
   return CoverageAndMutations<SymbolType>{coverage, mutations};
}

}  // namespace silo
