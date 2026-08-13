#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace rhydb {

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

/// A reference that is itself missing at some position makes a character that equals the
/// reference missing too, which the bulk comparison in extractCoverageAndMutationsFromSequence
/// must not skip over. This does not hold for the reference genomes we know of, so the check is
/// hoisted out of the per-sequence path.
template <typename SymbolType>
bool referenceContainsMissingSymbol(std::string_view reference) {
   return reference.find(SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING)) !=
          std::string_view::npos;
}

/// Diffs `sequence` (placed at `offset` within the genome) against `reference`, returning the
/// covered range, the missing (N) positions within it, and the mutations.
template <typename SymbolType>
std::expected<CoverageAndMutations<SymbolType>, std::string>
extractCoverageAndMutationsFromSequence(
   std::string_view sequence,
   size_t offset,
   std::string_view reference,
   bool reference_is_missing_somewhere
);

/// Convenience overload - prefer the 4-arg method
template <typename SymbolType>
std::expected<CoverageAndMutations<SymbolType>, std::string>
extractCoverageAndMutationsFromSequence(
   std::string_view sequence,
   size_t offset,
   std::string_view reference
);

}  // namespace rhydb
