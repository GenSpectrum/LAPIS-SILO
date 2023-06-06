#ifndef SILO_EXACT_H
#define SILO_EXACT_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

class Exact : public Expression {
   std::unique_ptr<Expression> child;

  public:
   explicit Exact(std::unique_ptr<Expression> child);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(invalid-case-style)
void from_json(const nlohmann::json& json, std::unique_ptr<Exact>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_EXACT_H
