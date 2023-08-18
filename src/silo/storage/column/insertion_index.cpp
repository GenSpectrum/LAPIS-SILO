#include "silo/storage/column/insertion_index.h"

#include <algorithm>
#include <iterator>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <boost/functional/hash.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/common/symbol_map.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column::insertion {

namespace {

constexpr std::string_view REGEX_ANY = ".*";

template <typename Symbol>
struct ThreeMerHash {
   size_t operator()(const std::array<Symbol, 3>& three_mer) const {
      size_t seed = 0;
      for (const auto one_mer : three_mer) {
         boost::hash_combine(seed, std::hash<Symbol>{}(one_mer));
      }
      return seed;
   }
};

template <typename Symbol>
std::vector<std::array<Symbol, 3>> extractThreeMers(const std::string& search_pattern) {
   std::unordered_set<std::array<Symbol, 3>, ThreeMerHash<Symbol>> result;
   for (const auto& continuous_string : splitBy(search_pattern, REGEX_ANY)) {
      auto continuous_symbols = Util<Symbol>::stringToSymbolVector(continuous_string);
      if (continuous_symbols == std::nullopt) {
         const auto illegal_nuc_char = Util<Symbol>::findIllegalChar(continuous_string);
         throw std::runtime_error(
            "Wrong symbol '" +
            (illegal_nuc_char.has_value() ? std::to_string(*illegal_nuc_char) : "Internal Error") +
            "' in pattern: " + search_pattern
         );
      }
      for (size_t i = 0; (i + 2) < continuous_string.size(); i += 3) {
         const std::array<Symbol, 3> three_mer{
            continuous_symbols->at(i),
            continuous_symbols->at(i + 1),
            continuous_symbols->at(i + 2)};
         result.insert(three_mer);
      }
   }
   return {result.begin(), result.end()};
}

}  // namespace

template <typename Symbol>
std::unique_ptr<roaring::Roaring> InsertionPosition<Symbol>::searchWithThreeMerIndex(
   const std::vector<std::array<Symbol, 3>>& search_three_mers,
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
      const auto& candidate_insertions =
         three_mer_index.at(three_mer[0]).at(three_mer[1]).at(three_mer[2]);
      if (candidate_insertions.empty()) {
         continue;
      }
      min_heap.emplace_back(candidate_insertions.cbegin(), candidate_insertions.cend());
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

template <typename Symbol>
std::unique_ptr<roaring::Roaring> InsertionPosition<Symbol>::searchWithRegex(
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
void InsertionPosition<NUCLEOTIDE_SYMBOL>::buildThreeMerIndex() {
   using ThreeMersBitset =
      NestedContainer<THREE_DIMENSIONS, SymbolMap, NUCLEOTIDE_SYMBOL, bool>::type;

   for (size_t insertion_id = 0; insertion_id < insertions.size(); ++insertion_id) {
      const auto& insertion_value = insertions[insertion_id].value;

      if (insertion_value.size() < 3) {
         continue;
      }

      const auto opt_nuc_symbol_ids =
         Util<NUCLEOTIDE_SYMBOL>::stringToSymbolVector(insertion_value);
      if (opt_nuc_symbol_ids == std::nullopt) {
         const auto illegal_nuc_char = Util<NUCLEOTIDE_SYMBOL>::findIllegalChar(insertion_value);
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

      for (const NUCLEOTIDE_SYMBOL symbol1 : Util<NUCLEOTIDE_SYMBOL>::symbols) {
         for (const NUCLEOTIDE_SYMBOL symbol2 : Util<NUCLEOTIDE_SYMBOL>::symbols) {
            for (const NUCLEOTIDE_SYMBOL symbol3 : Util<NUCLEOTIDE_SYMBOL>::symbols) {
               if (unique_three_mers[symbol1][symbol2][symbol3]) {
                  three_mer_index[symbol1][symbol2][symbol3].push_back(insertion_id);
               }
            }
         }
      }
   }
}

template <>
void InsertionPosition<AA_SYMBOL>::buildThreeMerIndex() {
   using ThreeMersBitset = NestedContainer<THREE_DIMENSIONS, SymbolMap, AA_SYMBOL, bool>::type;

   for (size_t insertion_id = 0; insertion_id < insertions.size(); ++insertion_id) {
      const auto& insertion_value = insertions[insertion_id].value;

      if (insertion_value.size() < 3) {
         continue;
      }

      const auto opt_nuc_symbol_ids = Util<AA_SYMBOL>::stringToSymbolVector(insertion_value);
      if (opt_nuc_symbol_ids == std::nullopt) {
         const auto illegal_nuc_char = Util<AA_SYMBOL>::findIllegalChar(insertion_value);
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

      for (const AA_SYMBOL symbol1 : Util<AA_SYMBOL>::symbols) {
         for (const AA_SYMBOL symbol2 : Util<AA_SYMBOL>::symbols) {
            for (const AA_SYMBOL symbol3 : Util<AA_SYMBOL>::symbols) {
               if (unique_three_mers[symbol1][symbol2][symbol3]) {
                  three_mer_index[symbol1][symbol2][symbol3].push_back(insertion_id);
               }
            }
         }
      }
   }
}

template <typename Symbol>
std::unique_ptr<roaring::Roaring> InsertionPosition<Symbol>::search(
   const std::string& search_pattern
) const {
   const auto search_three_mers = extractThreeMers<Symbol>(search_pattern);
   const std::regex regex_search_pattern(search_pattern);

   if (!search_three_mers.empty()) {
      // We can only use the ThreeMerIndex if there is at least one 3-mer in the search pattern
      return searchWithThreeMerIndex(search_three_mers, regex_search_pattern);
   }
   return searchWithRegex(regex_search_pattern);
}

template <typename Symbol>
void InsertionIndex<Symbol>::addLazily(
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

template <typename Symbol>
void InsertionIndex<Symbol>::buildIndex() {
   insertion_positions.reserve(collected_insertions.size());

   for (auto [pos, insertion_info] : collected_insertions) {
      InsertionPosition<Symbol> insertion_position;
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

template <typename Symbol>
std::unique_ptr<roaring::Roaring> InsertionIndex<Symbol>::search(
   uint32_t position,
   const std::string& search_pattern
) const {
   const auto insertion_pos_it = insertion_positions.find(position);
   if (insertion_pos_it == insertion_positions.end()) {
      return std::make_unique<roaring::Roaring>();
   }
   return insertion_pos_it->second.search(search_pattern);
}

template class InsertionIndex<NUCLEOTIDE_SYMBOL>;
template class InsertionIndex<AA_SYMBOL>;

}  // namespace silo::storage::column::insertion
