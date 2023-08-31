#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
struct Database;
struct DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

class Exact : public Expression {
   std::unique_ptr<Expression> child;

  public:
   explicit Exact(std::unique_ptr<Expression> child);

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Exact>& filter);

}  // namespace silo::query_engine::filter_expressions
