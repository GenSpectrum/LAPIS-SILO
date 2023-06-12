#ifndef SILO_OR_H
#define SILO_OR_H

#include <vector>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct Or : public Expression {
   std::vector<std::unique_ptr<Expression>> children;

  public:
   Or(std::vector<std::unique_ptr<Expression>>&& children);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Or>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_OR_H
