#ifndef SILO_MAYBE_H
#define SILO_MAYBE_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct Maybe : public Expression {
   std::unique_ptr<Expression> child;

   explicit Maybe(std::unique_ptr<Expression> child);

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Maybe>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_MAYBE_H
