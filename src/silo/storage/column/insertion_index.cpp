#include "silo/storage/column/insertion_index.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <boost/functional/hash.hpp>

#include "silo/common/numeric_conversion.h"
#include "silo/common/string_utils.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column::insertion {

namespace {

constexpr std::string_view DELIMITER_INSERTIONS = ",";
constexpr std::string_view DELIMITER_INSERTION = ":";
constexpr std::string_view REGEX_ANY = ".*";

std::pair<uint32_t, std::string> parseInsertion(const std::string& value) {
   const auto position_and_insertion = splitBy(value, DELIMITER_INSERTION);
   if (position_and_insertion.size() != 2) {
      const std::string message = "Failed to parse insertion due to invalid format: " + value;
      throw PreprocessingException(message);
   }
   const auto position = tryConvertStringToU32(position_and_insertion[0]);
   const auto& insertion = position_and_insertion[1];
   return std::make_pair(position, insertion);
}

struct ThreeMerHash {
   size_t operator()(const InsertionIndex::three_mer_t& three_mer) const {
      size_t seed = 0;
      for (const auto one_mer : three_mer) {
         boost::hash_combine(seed, std::hash<NUCLEOTIDE_SYMBOL>{}(one_mer));
      }
      return seed;
   }
};

std::vector<InsertionIndex::three_mer_t> extractThreeMers(const std::string& search_pattern) {
   std::unordered_set<InsertionIndex::three_mer_t, ThreeMerHash> result;
   for (const auto& continuous_string : splitBy(search_pattern, REGEX_ANY)) {
      auto continuous_symbols = stringToNucleotideSymbolVector(continuous_string);
      if (continuous_symbols == std::nullopt) {
         throw std::runtime_error("Wrong symbol in pattern: " + continuous_string);
      }
      for (size_t i = 0; (i + 2) < continuous_string.size(); i += 3) {
         InsertionIndex::three_mer_t const three_mer{
            continuous_symbols->at(i),
            continuous_symbols->at(i + 1),
            continuous_symbols->at(i + 2)};
         result.insert(three_mer);
      }
   }
   return {result.begin(), result.end()};
}

}  // namespace

void InsertionIndex::InsertionPosition::buildThreeMerIndex() {
   using bitset_one_mers_t = NucleotideSymbolMap<bool>;
   using bitset_two_mers_t = NucleotideSymbolMap<bitset_one_mers_t>;
   using bitset_three_mers_t = NucleotideSymbolMap<bitset_two_mers_t>;

   for (size_t insertion_id = 0; insertion_id < insertions.size(); ++insertion_id) {
      const auto& insertion = insertions[insertion_id];
      const auto& insertion_value = insertion.value;

      if (insertion_value.size() < 3) {
         continue;
      }

      const auto opt_nuc_symbol_ids = stringToNucleotideSymbolVector(insertion_value);
      if (opt_nuc_symbol_ids == std::nullopt) {
         throw silo::PreprocessingException("Illegal character in insertion: " + insertion_value);
      }
      const auto& nuc_symbol_ids = *opt_nuc_symbol_ids;

      bitset_three_mers_t unique_three_mers{};
      for (size_t i = 0; i < (nuc_symbol_ids.size() - 2); ++i) {
         unique_three_mers[nuc_symbol_ids[i]][nuc_symbol_ids[i + 1]][nuc_symbol_ids[i + 2]] = true;
      }

      for (const NUCLEOTIDE_SYMBOL symbol1 : NUC_SYMBOLS) {
         for (const NUCLEOTIDE_SYMBOL symbol2 : NUC_SYMBOLS) {
            for (const NUCLEOTIDE_SYMBOL symbol3 : NUC_SYMBOLS) {
               if (unique_three_mers[symbol1][symbol2][symbol3]) {
                  three_mer_index[symbol1][symbol2][symbol3].push_back(insertion_id);
               }
            }
         }
      }
   }
}

std::unique_ptr<InsertionIndex::sequence_ids_t> InsertionIndex::InsertionPosition::
   searchWithThreeMerIndex(
      const std::vector<three_mer_t>& search_three_mers,
      const std::regex& search_pattern
   ) const {
   assert(!search_three_mers.empty());

   // We perform a k-way intersection between the candidate sets of insertion ids.
   // The candidate insertions are selected based on the 3-mers within the search pattern.
   // If an insertion id is in all candidate sets, then we have a viable candidate which
   // might match the search pattern. If this does not hold, there exists a continuous
   // 3-mer which is in the search pattern but not in the candidate insertion. Therefore, the
   // regex will never match and we can ignore this sequence.

   using it = insertion_ids_t::const_iterator;
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
      return std::make_unique<sequence_ids_t>();
   }

   const auto cmp = [](const std::pair<it, it>& lhs_it_pair, const std::pair<it, it>& rhs_it_pair) {
      return *lhs_it_pair.first > *rhs_it_pair.first;
   };
   std::make_heap(min_heap.begin(), min_heap.end(), cmp);

   auto result = std::make_unique<sequence_ids_t>();

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

void InsertionIndex::addLazily(const std::string& insertions_string, uint32_t sequence_id) {
   if (insertions_string.empty()) {
      return;
   }
   for (auto& position_and_insertion : splitBy(insertions_string, DELIMITER_INSERTIONS)) {
      auto [position, insertion] = parseInsertion(position_and_insertion);

      auto it1 =
         collected_insertions.emplace(position, std::unordered_map<std::string, sequence_ids_t>{});
      auto it2 = it1.first->second.emplace(insertion, sequence_ids_t{});
      it2.first->second.add(sequence_id);
   }
}

void InsertionIndex::buildIndex() {
   insertion_positions.reserve(collected_insertions.size());

   for (auto [pos, insertion_info] : collected_insertions) {
      InsertionPosition insertion_position;
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

std::unique_ptr<roaring::Roaring> InsertionIndex::search(
   uint32_t position,
   const std::string& search_pattern
) const {
   const auto three_mers = extractThreeMers(search_pattern);
   const std::regex regex_search_pattern(search_pattern);

   const auto insertion_pos_it = insertion_positions.find(position);
   if (insertion_pos_it == insertion_positions.end()) {
      return std::make_unique<roaring::Roaring>();
   }

   if (!three_mers.empty()) {
      return insertion_pos_it->second.searchWithThreeMerIndex(
         three_mers, regex_search_pattern
      );
   }
   auto result = std::make_unique<roaring::Roaring>();
   for (const auto& insertion : insertion_pos_it->second.insertions) {
      if (std::regex_search(insertion.value, regex_search_pattern)) {
         *result |= insertion.sequence_ids;
      }
   }
   return result;
}

}  // namespace silo::storage::column::insertion
