#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter::expressions {

class IntBetween : public Expression {
  private:
   std::string column_name;
   std::optional<uint32_t> from;
   std::optional<uint32_t> to;

  public:
   explicit IntBetween(
      std::string column_name,
      std::optional<uint32_t> from,
      std::optional<uint32_t> to
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntBetween>& filter);

}  // namespace silo::query_engine::filter::expressions
