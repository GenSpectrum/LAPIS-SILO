#include "silo/query_engine/filter_expressions/nucleotide_symbol_maybe.h"

#include <vector>

#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/operator.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

NucleotideSymbolMaybe::NucleotideSymbolMaybe(unsigned int position, silo::NUCLEOTIDE_SYMBOL value)
    : position(position),
      value(value) {}

std::string NucleotideSymbolMaybe::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(position) + genomeSymbolRepresentation(value) + "?";
   return res;
}

std::unique_ptr<operators::Operator> NucleotideSymbolMaybe::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   auto symbols_to_match = AMBIGUITY_SYMBOLS.at(static_cast<uint64_t>(value));
   std::vector<std::unique_ptr<Expression>> symbol_filters;
   std::transform(
      symbols_to_match.begin(),
      symbols_to_match.end(),
      std::back_inserter(symbol_filters),
      [&](silo::NUCLEOTIDE_SYMBOL symbol) {
         return std::make_unique<NucleotideSymbolEquals>(position, symbol);
      }
   );
   return std::make_unique<Or>(std::move(symbol_filters))->compile(database, database_partition);
}

}  // namespace silo::query_engine::filter_expressions
