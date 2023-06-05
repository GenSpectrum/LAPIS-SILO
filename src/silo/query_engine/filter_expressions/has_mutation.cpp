#include "silo/query_engine/filter_expressions/has_mutation.h"

#include <vector>

#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"

#include "silo/database.h"

namespace silo::query_engine::filter_expressions {

HasMutation::HasMutation(uint32_t position)
    : position(position) {}

std::string HasMutation::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(position);
   return res;
}

std::unique_ptr<operators::Operator> HasMutation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   const char ref_symbol = database.global_reference[0].at(position);

   if (mode == UPPER_BOUND) {
      auto expression =
         std::make_unique<Negation>(std::make_unique<NucleotideSymbolEquals>(position, ref_symbol));
      return expression->compile(database, database_partition, NONE);
   }

   std::vector<NUCLEOTIDE_SYMBOL> symbols = {
      NUCLEOTIDE_SYMBOL::A,
      NUCLEOTIDE_SYMBOL::C,
      NUCLEOTIDE_SYMBOL::G,
      NUCLEOTIDE_SYMBOL::T,
   };
   (void)std::remove(symbols.begin(), symbols.end(), silo::toNucleotideSymbol(ref_symbol));
   std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
   std::transform(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbol_filters),
      [&](NUCLEOTIDE_SYMBOL symbol) {
         return std::make_unique<NucleotideSymbolEquals>(
            position, SYMBOL_REPRESENTATION[static_cast<uint32_t>(symbol)]
         );
      }
   );
   return Or(std::move(symbol_filters)).compile(database, database_partition, NONE);
}

void from_json(const nlohmann::json& json, std::unique_ptr<HasMutation>& filter) {
   CHECK_SILO_QUERY(
      json.contains("position"),
      "The field 'position' is required in a HasNucleotideMutation expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in a HasNucleotideMutation expression needs to be an unsigned "
      "integer"
   )
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   filter = std::make_unique<HasMutation>(position);
}

}  // namespace silo::query_engine::filter_expressions
