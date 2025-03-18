#pragma once

#include <memory>
#include <string>

#include <re2/re2.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class StringSearch : public Expression {
  private:
   std::string column_name;
   std::unique_ptr<re2::RE2> search_expression;

  public:
   explicit StringSearch(std::string column_name, std::unique_ptr<re2::RE2> search_expression);

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const Database& database,
      const storage::TablePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<StringSearch>& filter);

}  // namespace silo::query_engine::filter::expressions
