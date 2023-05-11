#ifndef SILO_AND_H
#define SILO_AND_H

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct And : public Expression {
   std::vector<std::unique_ptr<Expression>> children;

   explicit And(std::vector<std::unique_ptr<Expression>>&& children);

   std::string toString(const silo::Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_AND_H
