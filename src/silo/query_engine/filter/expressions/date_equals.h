#pragma once

#include <memory>
#include <optional>
#include <string>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

class DateEquals : public Expression {
  private:
   std::string column_name;
   std::optional<silo::common::Date32> value;

  public:
   explicit DateEquals(std::string column_name, std::optional<silo::common::Date32> value);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::filter::expressions
