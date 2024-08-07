#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class DatabasePartition;

namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
class Database;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

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

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntBetween>& filter);

}  // namespace silo::query_engine::filter_expressions
