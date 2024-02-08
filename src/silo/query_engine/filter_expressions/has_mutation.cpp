#include "silo/query_engine/filter_expressions/has_mutation.h"

#include <map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/filter_expressions/symbol_equals.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo {
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

template <typename SymbolType>
HasMutation<SymbolType>::HasMutation(std::optional<std::string> sequence_name, uint32_t position)
    : sequence_name(std::move(sequence_name)),
      position(position) {}

template <typename SymbolType>
std::string HasMutation<SymbolType>::toString(const silo::Database& /*database*/) const {
   const std::string sequence_name_prefix = sequence_name ? sequence_name.value() + ":" : "";
   return sequence_name_prefix + std::to_string(position);
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> HasMutation<SymbolType>::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || database.getDefaultSequenceName<SymbolType>().has_value(),
      fmt::format(
         "Database does not have a default sequence name for {} Sequences", SymbolType::SYMBOL_NAME
      )
   );
   const std::string sequence_name_or_default =
      sequence_name.has_value() ? sequence_name.value()
                                : database.getDefaultSequenceName<SymbolType>().value();
   CHECK_SILO_QUERY(
      database.getSequenceStores<SymbolType>().contains(sequence_name_or_default),
      fmt::format(
         "Database does not contain the {} sequence with name: '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name_or_default
      )
   )

   auto ref_symbol = database.getSequenceStores<SymbolType>()
                        .at(sequence_name_or_default)
                        .reference_sequence.at(position);

   std::vector<typename SymbolType::Symbol> symbols =
      std::vector(SymbolType::SYMBOLS.begin(), SymbolType::SYMBOLS.end());
   if (mode == AmbiguityMode::UPPER_BOUND) {
      // We can only be sure, that the symbol did not mutate, if the ref_symbol is at that position
      symbols.erase(std::remove(symbols.begin(), symbols.end(), ref_symbol), symbols.end());
   } else {
      // Remove all symbols that could match the searched base
      for (const auto symbol : SymbolType::AMBIGUITY_SYMBOLS.at(ref_symbol)) {
         symbols.erase(std::remove(symbols.begin(), symbols.end(), symbol), symbols.end());
      }
   }
   std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
   std::transform(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbol_filters),
      [&](typename SymbolType::Symbol symbol) {
         return std::make_unique<SymbolEquals<SymbolType>>(
            sequence_name_or_default, position, symbol
         );
      }
   );
   return Or(std::move(symbol_filters)).compile(database, database_partition, NONE);
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasMutation<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.contains("position"),
      "The field 'position' is required in a HasNucleotideMutation expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in a HasNucleotideMutation expression needs to be an unsigned integer"
   )
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   filter = std::make_unique<HasMutation<SymbolType>>(nuc_sequence_name, position);
}

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<HasMutation<Nucleotide>>& filter
);

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<HasMutation<AminoAcid>>& filter
);

}  // namespace silo::query_engine::filter_expressions
