#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class DatabasePartition;
namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
struct Database;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

struct Or : public Expression {
   std::vector<std::unique_ptr<Expression>> children;

  public:
   Or(std::vector<std::unique_ptr<Expression>>&& children);

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Or>& filter);

}  // namespace silo::query_engine::filter_expressions
