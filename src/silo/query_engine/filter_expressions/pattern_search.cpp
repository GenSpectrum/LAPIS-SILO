#include "silo/query_engine/filter_expressions/pattern_search.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

PatternSearch::PatternSearch(
   std::optional<std::string> nuc_sequence_name,
   uint32_t position,
   std::vector<NUCLEOTIDE_SYMBOL> pattern
)
    : nuc_sequence_name(std::move(nuc_sequence_name)),
      position(position),
      pattern(std::move(pattern)) {}

std::string PatternSearch::toString(const silo::Database& /*database*/) const {
   const std::string nuc_sequence_name_prefix =
      nuc_sequence_name ? nuc_sequence_name.value() + ":" : "";
   std::vector<char> pattern_chars;
   std::transform(pattern.begin(), pattern.end(), pattern_chars.begin(), nucleotideSymbolToChar);
   return nuc_sequence_name_prefix + std::to_string(position + 1) +
          std::string(pattern_chars.data());
}

std::unique_ptr<silo::query_engine::operators::Operator> PatternSearch::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )

   // NOLINTNEXTLINE(clang-diagnostic-unused-variable)
   const auto& seq_store_partition =
      database_partition.nuc_sequences.at(nuc_sequence_name_or_default);

   // TODO(#165) use the data structures from your adapted sequence store to build an operator tree
   //            that answers the query

   return std::make_unique<operators::Empty>(database_partition.sequenceCount);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PatternSearch>& filter) {
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a PatternSearch expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && json["position"].get<uint32_t>() > 0,
      "The field 'position' in a PatternSearch expression needs to be an unsigned "
      "integer greater than 0"
   )
   CHECK_SILO_QUERY(
      json.contains("pattern") && json["pattern"].is_string(),
      "The string field 'pattern' is required in a PatternSearch expression"
   )
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   const std::string pattern_chars = json["pattern"].get<std::string>();

   std::vector<NUCLEOTIDE_SYMBOL> pattern;
   std::transform(
      pattern_chars.begin(),
      pattern_chars.end(),
      pattern.begin(),
      [&pattern_chars](char character) {
         auto symbol = charToNucleotideSymbol(character);
         CHECK_SILO_QUERY(
            symbol.has_value(),
            "The pattern " + pattern_chars + " contains the illegal character " +
               std::to_string(character)
         )
         return *symbol;
      }
   );

   filter = std::make_unique<PatternSearch>(nuc_sequence_name, position, pattern);
}

}  // namespace silo::query_engine::filter_expressions
