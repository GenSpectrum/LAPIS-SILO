#include "silo/query_engine/filter_expressions/has_aa_mutation.h"

#include <array>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/aa_symbol_equals.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo {
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

HasAAMutation::HasAAMutation(std::string aa_sequence_name, uint32_t position_idx)
    : aa_sequence_name(std::move(aa_sequence_name)),
      position_idx(position_idx) {}

std::string HasAAMutation::toString(const silo::Database& /*database*/) const {
   std::string res = aa_sequence_name + ":" + std::to_string(position_idx);
   return res;
}

std::unique_ptr<operators::Operator> HasAAMutation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   const AminoAcid::Symbol ref_symbol =
      database.aa_sequences.at(aa_sequence_name).reference_sequence.at(position_idx);

   if (mode == UPPER_BOUND) {
      auto expression = std::make_unique<Negation>(
         std::make_unique<AASymbolEquals>(aa_sequence_name, position_idx, ref_symbol)
      );
      return expression->compile(database, database_partition, NONE);
   }

   std::vector<AminoAcid::Symbol> symbols(AminoAcid::SYMBOLS.begin(), AminoAcid::SYMBOLS.end());
   // NOLINTNEXTLINE(bugprone-unused-return-value)
   (void)std::remove(symbols.begin(), symbols.end(), AminoAcid::Symbol::X);
   // NOLINTNEXTLINE(bugprone-unused-return-value)
   (void)std::remove(symbols.begin(), symbols.end(), ref_symbol);
   std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
   std::transform(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbol_filters),
      [&](AminoAcid::Symbol symbol) {
         return std::make_unique<AASymbolEquals>(aa_sequence_name, position_idx, symbol);
      }
   );
   return Or(std::move(symbol_filters)).compile(database, database_partition, NONE);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasAAMutation>& filter) {
   CHECK_SILO_QUERY(
      json.contains("position"),
      "The field 'position' is required in a HasAminoAcidMutation expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in a HasAminoAcidMutation expression needs to be an unsigned integer"
   )
   CHECK_SILO_QUERY(
      json.contains("sequenceName") && json["sequenceName"].is_string(),
      "HasAminoAcidMutation expression requires the string field sequenceName"
   )
   const std::string aa_sequence_name = json["sequenceName"].get<std::string>();
   const uint32_t position_idx = json["position"].get<uint32_t>() - 1;
   filter = std::make_unique<HasAAMutation>(aa_sequence_name, position_idx);
}

}  // namespace silo::query_engine::filter_expressions
