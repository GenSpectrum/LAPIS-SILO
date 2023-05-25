#ifndef SILO_NEGATION_H
#define SILO_NEGATION_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct Negation : public Expression {
   std::unique_ptr<Expression> child;

   explicit Negation(std::unique_ptr<Expression> child);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_NEGATION_H
