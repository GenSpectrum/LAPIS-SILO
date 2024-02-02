#include "silo/storage/column/insertion_column.h"

#include <optional>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/common/bidirectional_map.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/string_utils.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/column/insertion_index.h"

namespace silo::storage::column {

namespace {

constexpr std::string_view DELIMITER_INSERTIONS = ",";
constexpr std::string_view DELIMITER_INSERTION = ":";

struct InsertionEntry {
   std::string sequence_name;
   uint32_t position;
   std::string insertion;
};

InsertionEntry parseInsertion(
   const std::string& value,
   const std::optional<std::string>& default_sequence_name
) {
   auto position_and_insertion = splitBy(value, DELIMITER_INSERTION);
   std::transform(
      position_and_insertion.begin(),
      position_and_insertion.end(),
      position_and_insertion.begin(),
      [](const std::string& value) { return silo::removeSymbol(value, '\"'); }
   );
   try {
      if (position_and_insertion.size() == 2) {
         if (default_sequence_name == std::nullopt) {
            const std::string message = "Failed to parse insertion due to invalid format: " + value;
            throw preprocessing::PreprocessingException(message);
         }
         const auto position = boost::lexical_cast<uint32_t>(position_and_insertion[0]);
         const auto& insertion = position_and_insertion[1];
         return {*default_sequence_name, position, insertion};
      }
      if (position_and_insertion.size() == 3) {
         const auto& sequence_name = position_and_insertion[0];
         const auto position = boost::lexical_cast<uint32_t>(position_and_insertion[1]);
         const auto& insertion = position_and_insertion[2];
         return {sequence_name, position, insertion};
      }
   } catch (const boost::bad_lexical_cast& error) {
      const std::string message = "Failed to parse insertion due to invalid format: " + value;
      throw preprocessing::PreprocessingException(message + ". Error: " + error.what());
   }

   const std::string message = "Failed to parse insertion due to invalid format: " + value;
   throw preprocessing::PreprocessingException(message);
}
}  // namespace

template <typename SymbolType>
InsertionColumnPartition<SymbolType>::InsertionColumnPartition(
   common::BidirectionalMap<std::string>& lookup,
   std::optional<std::string> default_sequence_name
)
    : lookup(lookup),
      default_sequence_name(std::move(default_sequence_name)) {}

template <typename SymbolType>
void InsertionColumnPartition<SymbolType>::insert(const std::string& value) {
   if (value.empty()) {
      insertNull();
      return;
   }
   const auto sequence_id = values.size();

   std::string standardized_value;
   for (auto& insertion_entry : splitBy(value, DELIMITER_INSERTIONS)) {
      auto [sequence_name, position, insertion] =
         parseInsertion(insertion_entry, default_sequence_name);
      auto& insertion_index =
         insertion_indexes.emplace(sequence_name, insertion::InsertionIndex<SymbolType>{})
            .first->second;
      insertion_index.addLazily(position, insertion, sequence_id);

      if (!standardized_value.empty()) {
         standardized_value += DELIMITER_INSERTIONS;
      }
      if (default_sequence_name.has_value() && default_sequence_name == sequence_name) {
         standardized_value += fmt::format("{}{}{}", position, DELIMITER_INSERTION, insertion);
      } else {
         standardized_value += fmt::format(
            "{}{}{}{}{}",
            sequence_name,
            DELIMITER_INSERTION,
            position,
            DELIMITER_INSERTION,
            insertion
         );
      }
   }

   const Idx value_id = lookup.getOrCreateId(standardized_value);
   values.push_back(value_id);
}

template <typename SymbolType>
void InsertionColumnPartition<SymbolType>::insertNull() {
   const Idx value_id = lookup.getOrCreateId("");
   values.push_back(value_id);
}

template <typename SymbolType>
void InsertionColumnPartition<SymbolType>::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

template <typename SymbolType>
void InsertionColumnPartition<SymbolType>::buildInsertionIndexes() {
   for (auto& [_, insertion_index] : insertion_indexes) {
      insertion_index.buildIndex();
   }
}

template <typename SymbolType>
const std::unordered_map<std::string, insertion::InsertionIndex<SymbolType>>&
InsertionColumnPartition<SymbolType>::getInsertionIndexes() const {
   return insertion_indexes;
}

template <typename SymbolType>
std::unique_ptr<roaring::Roaring> InsertionColumnPartition<SymbolType>::search(
   const std::string& sequence_name,
   uint32_t position,
   const std::string& search_pattern
) const {
   if (!insertion_indexes.contains(sequence_name)) {
      return std::make_unique<roaring::Roaring>();
   }
   return insertion_indexes.at(sequence_name).search(position, search_pattern);
}

template <typename SymbolType>
const std::vector<silo::Idx>& InsertionColumnPartition<SymbolType>::getValues() const {
   return values;
}

template <typename SymbolType>
std::string InsertionColumnPartition<SymbolType>::lookupValue(silo::Idx value_id) const {
   return lookup.getValue(value_id);
}

template <typename SymbolType>
InsertionColumn<SymbolType>::InsertionColumn(std::optional<std::string> default_sequence_name)
    : default_sequence_name(std::move(default_sequence_name)) {
   lookup = std::make_unique<silo::common::BidirectionalMap<std::string>>();
}

template <typename SymbolType>
InsertionColumnPartition<SymbolType>& InsertionColumn<SymbolType>::createPartition() {
   return partitions.emplace_back(*lookup, default_sequence_name);
}

template class InsertionColumnPartition<AminoAcid>;
template class InsertionColumnPartition<Nucleotide>;
template class InsertionColumn<AminoAcid>;
template class InsertionColumn<Nucleotide>;

}  // namespace silo::storage::column
