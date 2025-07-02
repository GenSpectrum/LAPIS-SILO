#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class NOf : public Expression {
  private:
   ExpressionVector children;
   int number_of_matchers;
   bool match_exactly;

   std::tuple<operators::OperatorVector, operators::OperatorVector, int> mapChildExpressions(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const;

   std::unique_ptr<operators::Operator> rewriteNonExact(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      Expression::AmbiguityMode mode
   ) const;

  public:
   explicit NOf(ExpressionVector&& children, int number_of_matchers, bool match_exactly);

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter);

}  // namespace silo::query_engine::filter::expressions
