#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class False : public Expression {
  public:
   explicit False();

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const Database& database,
      const storage::TablePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<False>& filter);

}  // namespace silo::query_engine::filter::expressions
