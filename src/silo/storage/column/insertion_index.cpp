#include "silo/storage/column/insertion_index.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <boost/container_hash/hash.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column::insertion {

constexpr std::string_view REGEX_ANY = ".*";

template <typename SymbolType>
size_t ThreeMerHash<SymbolType>::operator()(
   const std::array<typename SymbolType::Symbol, 3>& three_mer
) const {
   size_t seed = 0;
   for (const auto one_mer : three_mer) {
      boost::hash_combine(seed, std::hash<typename SymbolType::Symbol>{}(one_mer));
   }
   return seed;
}

template <typename SymbolType>
std::vector<std::array<typename SymbolType::Symbol, 3>> extractThreeMers(
   const std::string& search_pattern
) {
   std::unordered_set<std::array<typename SymbolType::Symbol, 3>, ThreeMerHash<SymbolType>> result;
   for (const auto& continuous_string : splitBy(search_pattern, REGEX_ANY)) {
      auto continuous_symbols = SymbolType::stringToSymbolVector(continuous_string);
      if (continuous_symbols == std::nullopt) {
         const auto illegal_nuc_char = SymbolType::findIllegalChar(continuous_string);
         throw std::runtime_error(
            "Wrong symbol '" +
            (illegal_nuc_char.has_value() ? std::to_string(*illegal_nuc_char) : "Internal Error") +
            "' in pattern: " + search_pattern
         );
      }
      for (size_t i = 0; (i + 2) < continuous_string.size(); i += 3) {
         const std::array<typename SymbolType::Symbol, 3> three_mer{
            continuous_symbols->at(i),
            continuous_symbols->at(i + 1),
            continuous_symbols->at(i + 2)};
         result.insert(three_mer);
      }
   }
   return {result.begin(), result.end()};
}

template <typename SymbolType>
std::unique_ptr<roaring::Roaring> InsertionPosition<SymbolType>::searchWithThreeMerIndex(
   const std::vector<std::array<typename SymbolType::Symbol, 3>>& search_three_mers,
   const std::regex& search_pattern
) const {
   // We perform a k-way intersection between the candidate sets of insertion ids.
   // The candidate insertions are selected based on the 3-mers within the search pattern.
   // If an insertion id is in all candidate sets, then we have a viable candidate which
   // might match the search pattern. If this does not hold, there exists a continuous
   // 3-mer which is in the search pattern but not in the candidate insertion. Therefore, the
   // regex will never match and we can ignore this sequence.

   using it = InsertionIds::const_iterator;
   std::vector<std::pair<it, it>> min_heap;
   for (const auto& three_mer : search_three_mers) {
      if (three_mer_index.contains(three_mer)) {
         const auto& candidate_insertions = three_mer_index.at(three_mer);
         min_heap.emplace_back(candidate_insertions.cbegin(), candidate_insertions.cend());
      }
   }

   if (min_heap.size() < search_three_mers.size()) {
      return std::make_unique<roaring::Roaring>();
   }

   const auto cmp = [](const std::pair<it, it>& lhs_it_pair, const std::pair<it, it>& rhs_it_pair) {
      return *lhs_it_pair.first > *rhs_it_pair.first;
   };
   std::make_heap(min_heap.begin(), min_heap.end(), cmp);

   auto result = std::make_unique<roaring::Roaring>();

   size_t count = 0;
   uint32_t current_insertion_id = *min_heap.front().first;
   while (!min_heap.empty()) {
      const auto next_insertion_id = *min_heap.front().first;

      std::pop_heap(min_heap.begin(), min_heap.end(), cmp);
      ++min_heap.back().first;

      if (min_heap.back().first == min_heap.back().second) {
         min_heap.pop_back();
      } else {
         std::push_heap(min_heap.begin(), min_heap.end(), cmp);
      }

      if (next_insertion_id != current_insertion_id) {
         if (count == search_three_mers.size()) {
            const auto& insertion = insertions[current_insertion_id];
            if (std::regex_search(insertion.value, search_pattern)) {
               *result |= insertion.sequence_ids;
            }
         }
         count = 1;
         current_insertion_id = next_insertion_id;
      } else {
         ++count;
      }
   }

   if (count == search_three_mers.size()) {
      const auto& insertion = insertions[current_insertion_id];
      if (std::regex_search(insertion.value, search_pattern)) {
         *result |= insertion.sequence_ids;
      }
   }

   return result;
}

template <typename SymbolType>
std::unique_ptr<roaring::Roaring> InsertionPosition<SymbolType>::searchWithRegex(
   const std::regex& regex_search_pattern
) const {
   auto result = std::make_unique<roaring::Roaring>();
   for (const auto& insertion : insertions) {
      if (std::regex_search(insertion.value, regex_search_pattern)) {
         *result |= insertion.sequence_ids;
      }
   }
   return result;
}

template <>
void InsertionPosition<Nucleotide>::buildThreeMerIndex() {
   using ThreeMersBitset =
      SymbolMap<Nucleotide, SymbolMap<Nucleotide, SymbolMap<Nucleotide, bool>>>;

   for (size_t insertion_id = 0; insertion_id < insertions.size(); ++insertion_id) {
      const auto& insertion_value = insertions[insertion_id].value;

      if (insertion_value.size() < 3) {
         continue;
      }

      const auto opt_nuc_symbol_ids = Nucleotide::stringToSymbolVector(insertion_value);
      if (opt_nuc_symbol_ids == std::nullopt) {
         const auto illegal_nuc_char = Nucleotide::findIllegalChar(insertion_value);
         throw silo::PreprocessingException(
            "Illegal nucleotide character '" +
            (illegal_nuc_char.has_value() ? std::to_string(*illegal_nuc_char) : "Internal Error") +
            "' in insertion: " + insertion_value
         );
      }
      const auto& nuc_symbol_ids = *opt_nuc_symbol_ids;

      ThreeMersBitset unique_three_mers{};
      for (size_t i = 0; i < (nuc_symbol_ids.size() - 2); ++i) {
         unique_three_mers[nuc_symbol_ids[i]][nuc_symbol_ids[i + 1]][nuc_symbol_ids[i + 2]] = true;
      }

      for (const Nucleotide::Symbol symbol1 : Nucleotide::SYMBOLS) {
         for (const Nucleotide::Symbol symbol2 : Nucleotide::SYMBOLS) {
            for (const Nucleotide::Symbol symbol3 : Nucleotide::SYMBOLS) {
               if (unique_three_mers[symbol1][symbol2][symbol3]) {
                  std::array<Nucleotide::Symbol, 3> tuple{symbol1, symbol2, symbol3};
                  three_mer_index.emplace(tuple, InsertionIds{})
                     .first->second.push_back(insertion_id);
               }
            }
         }
      }
   }
}

template <>
void InsertionPosition<AminoAcid>::buildThreeMerIndex() {
   using ThreeMersBitset = SymbolMap<AminoAcid, SymbolMap<AminoAcid, SymbolMap<AminoAcid, bool>>>;

   for (size_t insertion_id = 0; insertion_id < insertions.size(); ++insertion_id) {
      const auto& insertion_value = insertions[insertion_id].value;

      if (insertion_value.size() < 3) {
         continue;
      }

      const auto opt_aa_symbol_ids = AminoAcid::stringToSymbolVector(insertion_value);
      if (opt_aa_symbol_ids == std::nullopt) {
         const auto illegal_aa_char = AminoAcid::findIllegalChar(insertion_value);
         throw silo::PreprocessingException(
            "Illegal amino acid character '" +
            (illegal_aa_char.has_value() ? std::to_string(*illegal_aa_char) : "Internal Error") +
            "' in insertion: " + insertion_value
         );
      }
      const auto& aa_symbol_ids = *opt_aa_symbol_ids;

      ThreeMersBitset unique_three_mers{};
      for (size_t i = 0; i < (aa_symbol_ids.size() - 2); ++i) {
         unique_three_mers[aa_symbol_ids[i]][aa_symbol_ids[i + 1]][aa_symbol_ids[i + 2]] = true;
      }

      for (const AminoAcid::Symbol symbol1 : AminoAcid::SYMBOLS) {
         for (const AminoAcid::Symbol symbol2 : AminoAcid::SYMBOLS) {
            for (const AminoAcid::Symbol symbol3 : AminoAcid::SYMBOLS) {
               if (unique_three_mers[symbol1][symbol2][symbol3]) {
                  std::array<AminoAcid::Symbol, 3> tuple{symbol1, symbol2, symbol3};
                  three_mer_index.emplace(tuple, InsertionIds{})
                     .first->second.push_back(insertion_id);
               }
            }
         }
      }
   }
}

template <typename SymbolType>
std::unique_ptr<roaring::Roaring> InsertionPosition<SymbolType>::search(
   const std::string& search_pattern
) const {
   const auto search_three_mers = extractThreeMers<SymbolType>(search_pattern);
   const std::regex regex_search_pattern(search_pattern);

   if (!search_three_mers.empty()) {
      // We can only use the ThreeMerIndex if there is at least one 3-mer in the search pattern
      return searchWithThreeMerIndex(search_three_mers, regex_search_pattern);
   }
   return searchWithRegex(regex_search_pattern);
}

template <typename SymbolType>
void InsertionIndex<SymbolType>::addLazily(
   uint32_t position,
   const std::string& insertion,
   uint32_t sequence_id
) {
   auto& insertions_at_position =
      collected_insertions.emplace(position, std::unordered_map<std::string, roaring::Roaring>{})
         .first->second;
   auto& sequence_ids_at_position =
      insertions_at_position.emplace(insertion, roaring::Roaring{}).first->second;
   sequence_ids_at_position.add(sequence_id);
}

template <typename SymbolType>
void InsertionIndex<SymbolType>::buildIndex() {
   insertion_positions.reserve(collected_insertions.size());

   for (auto [pos, insertion_info] : collected_insertions) {
      InsertionPosition<SymbolType> insertion_position;
      insertion_position.insertions.reserve(insertion_info.size());
      std::transform(
         insertion_info.begin(),
         insertion_info.end(),
         std::back_inserter(insertion_position.insertions),
         [](auto& insertion) {
            return Insertion{std::move(insertion.first), std::move(insertion.second)};
         }
      );
      insertion_position.buildThreeMerIndex();
      insertion_positions.emplace(pos, std::move(insertion_position));
   }

   // free up the memory
   collected_insertions.clear();
}

template <typename SymbolType>
const std::unordered_map<uint32_t, InsertionPosition<SymbolType>>& InsertionIndex<
   SymbolType>::getInsertionPositions() const {
   return insertion_positions;
}

template <typename SymbolType>
std::unique_ptr<roaring::Roaring> InsertionIndex<SymbolType>::search(
   uint32_t position,
   const std::string& search_pattern
) const {
   const auto insertion_pos_it = insertion_positions.find(position);
   if (insertion_pos_it == insertion_positions.end()) {
      return std::make_unique<roaring::Roaring>();
   }
   return insertion_pos_it->second.search(search_pattern);
}

template class ThreeMerHash<Nucleotide>;
template class ThreeMerHash<AminoAcid>;

template class InsertionIndex<Nucleotide>;
template class InsertionIndex<AminoAcid>;

}  // namespace silo::storage::column::insertion
