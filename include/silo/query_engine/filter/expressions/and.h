#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter::expressions {

class And : public Expression {
  private:
   std::vector<std::unique_ptr<Expression>> children;

   std::tuple<
      std::vector<std::unique_ptr<operators::Operator>>,
      std::vector<std::unique_ptr<operators::Operator>>,
      std::vector<std::unique_ptr<operators::Predicate>>>
   compileChildren(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const;

  public:
   explicit And(std::vector<std::unique_ptr<Expression>>&& children);

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<And>& filter);

}  // namespace silo::query_engine::filter::expressions
