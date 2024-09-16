#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

class NOf : public Expression {
  private:
   std::vector<std::unique_ptr<Expression>> children;
   int number_of_matchers;
   bool match_exactly;

   std::tuple<
      std::vector<std::unique_ptr<operators::Operator>>,
      std::vector<std::unique_ptr<operators::Operator>>,
      int>
   mapChildExpressions(
      const silo::Database& database,
      const silo::DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const;

   std::unique_ptr<operators::Operator> rewriteNonExact(
      const silo::Database& database,
      const silo::DatabasePartition& database_partition,
      Expression::AmbiguityMode mode
   ) const;

  public:
   explicit NOf(
      std::vector<std::unique_ptr<Expression>>&& children,
      int number_of_matchers,
      bool match_exactly
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter);

}  // namespace silo::query_engine::filter_expressions
