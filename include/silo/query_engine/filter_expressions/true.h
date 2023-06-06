#ifndef SILO_TRUE_H
#define SILO_TRUE_H

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct True : public Expression {
   explicit True();

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(invalid-case-style)
void from_json(const nlohmann::json& json, std::unique_ptr<True>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_TRUE_H
