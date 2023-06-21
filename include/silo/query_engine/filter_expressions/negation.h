#ifndef SILO_NEGATION_H
#define SILO_NEGATION_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

class Negation : public Expression {
  private:
   std::unique_ptr<Expression> child;

  public:
   explicit Negation(std::unique_ptr<Expression> child);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Negation>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_NEGATION_H