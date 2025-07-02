#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class Expression {
  protected:
   Expression();

  public:
   virtual ~Expression() = default;

   /// UPPER_BOUND returns the upper bound of sequences matching this expression (i.e. ambiguous
   /// codes count as matches), LOWER_BOUND returns the lower bound of sequences matching this
   /// expression (i.e. ambiguous codes in negations count as matches)
   /// NONE does not specially consider ambiguous symbols
   enum AmbiguityMode { UPPER_BOUND, LOWER_BOUND, NONE };

   virtual std::string toString() const = 0;

   [[nodiscard]] virtual std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const = 0;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Expression>& filter);

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode);

using ExpressionVector = std::vector<std::unique_ptr<Expression>>;

}  // namespace silo::query_engine::filter::expressions
