#ifndef SILO_NUCLEOTIDE_SYMBOL_MAYBE_H
#define SILO_NUCLEOTIDE_SYMBOL_MAYBE_H

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct NucleotideSymbolMaybe : public Expression {
   unsigned position;
   NUCLEOTIDE_SYMBOL value;

   explicit NucleotideSymbolMaybe(unsigned position, NUCLEOTIDE_SYMBOL value);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_NUCLEOTIDE_SYMBOL_MAYBE_H
