#ifndef SILO_NUCLEOTIDE_SYMBOL_EQUALS_H
#define SILO_NUCLEOTIDE_SYMBOL_EQUALS_H

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct NucleotideSymbolEquals : public Expression {
   unsigned position;
   char value;

   explicit NucleotideSymbolEquals(unsigned position, char value);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(invalid-case-style)
void from_json(const nlohmann::json& json, std::unique_ptr<NucleotideSymbolEquals>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_NUCLEOTIDE_SYMBOL_EQUALS_H
