#ifndef SILO_FALSE_H
#define SILO_FALSE_H

#include <memory>
#include <string>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine::filter_expressions {

struct False : public Expression {
   explicit False();

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<False>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_FALSE_H