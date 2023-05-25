#include "silo/query_engine/filter_expressions/has_mutation.h"

#include <vector>

#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/operator.h"

#include "silo/database.h"

namespace silo::query_engine::filter_expressions {

HasMutation::HasMutation(unsigned int position)
    : position(position) {}

std::string HasMutation::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(position);
   return res;
}

std::unique_ptr<operators::Operator> HasMutation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition
) const {
   const char ref_symbol = database.global_reference[0].at(position);
   std::vector<NUCLEOTIDE_SYMBOL> symbols = {
      NUCLEOTIDE_SYMBOL::A,
      NUCLEOTIDE_SYMBOL::C,
      NUCLEOTIDE_SYMBOL::G,
      NUCLEOTIDE_SYMBOL::T,
   };
   std::remove(symbols.begin(), symbols.end(), silo::toNucleotideSymbol(ref_symbol));
   std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
   std::transform(
      symbols.begin(),
      symbols.end(),
      std::back_inserter(symbol_filters),
      [&](NUCLEOTIDE_SYMBOL symbol) {
         return std::make_unique<NucleotideSymbolEquals>(position, symbol);
      }
   );
   return Or(std::move(symbol_filters)).compile(database, database_partition);
}

}  // namespace silo::query_engine::filter_expressions
