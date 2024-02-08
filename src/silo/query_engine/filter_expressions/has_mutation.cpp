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
#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo {
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

HasMutation::HasMutation(std::optional<std::string> nuc_sequence_name, uint32_t position_idx)
    : nuc_sequence_name(std::move(nuc_sequence_name)),
      position_idx(position_idx) {}

std::string HasMutation::toString(const silo::Database& /*database*/) const {
   const std::string nuc_sequence_name_prefix =
      nuc_sequence_name ? nuc_sequence_name.value() + ":" : "";
   return nuc_sequence_name_prefix + std::to_string(position_idx);
}

std::unique_ptr<operators::Operator> HasMutation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )

   const Nucleotide::Symbol ref_symbol =
      database.nuc_sequences.at(nuc_sequence_name_or_default).reference_sequence.at(position_idx);

   if (mode == UPPER_BOUND) {
      auto expression = std::make_unique<Negation>(std::make_unique<NucleotideSymbolEquals>(
         nuc_sequence_name_or_default, position_idx, ref_symbol
      ));
      return expression->compile(database, database_partition, NONE);
   }

   std::vector<Nucleotide::Symbol> symbols = {
      Nucleotide::Symbol::A,
      Nucleotide::Symbol::C,
      Nucleotide::Symbol::G,
      Nucleotide::Symbol::T,
   };
   // NOLINTNEXTLINE(bugprone-unused-return-value)
   (void)std::remove(symbols.begin(), symbols.end(), ref_symbol);
   std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
   std::transform(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbol_filters),
      [&](Nucleotide::Symbol symbol) {
         return std::make_unique<NucleotideSymbolEquals>(
            nuc_sequence_name_or_default, position_idx, symbol
         );
      }
   );
   return Or(std::move(symbol_filters)).compile(database, database_partition, NONE);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasMutation>& filter) {
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
   const uint32_t position_idx = json["position"].get<uint32_t>() - 1;
   filter = std::make_unique<HasMutation>(nuc_sequence_name, position_idx);
}

}  // namespace silo::query_engine::filter_expressions
