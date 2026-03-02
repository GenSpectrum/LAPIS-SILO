#pragma once

#include <memory>
#include <string>
#include <unordered_set>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class Or;

class StringInSet : public Expression {
   friend class Or;

  private:
   std::string column_name;
   std::unordered_set<std::string> values;

  public:
   explicit StringInSet(std::string column_name, std::unordered_set<std::string> value);

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

}  // namespace silo::query_engine::filter::expressions
