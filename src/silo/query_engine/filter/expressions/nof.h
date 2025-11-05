#pragma once

#include <memory>
#include <string>
#include <tuple>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class NOf : public Expression {
  private:
   ExpressionVector children;
   int number_of_matchers;
   bool match_exactly;

   [[nodiscard]] ExpressionVector rewriteChildren(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      Expression::AmbiguityMode mode
   ) const;

   [[nodiscard]] std::unique_ptr<Expression> rewriteToNonExact(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      Expression::AmbiguityMode mode
   ) const;

   [[nodiscard]] std::tuple<operators::OperatorVector, operators::OperatorVector, int>
   mapChildExpressions(const storage::Table& table, const storage::TablePartition& table_partition)
      const;

  public:
   explicit NOf(ExpressionVector&& children, int number_of_matchers, bool match_exactly);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NOf>& filter);

}  // namespace silo::query_engine::filter::expressions
