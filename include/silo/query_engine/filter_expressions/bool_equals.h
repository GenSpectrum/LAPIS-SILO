#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/optional_bool.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class Database;
class DatabasePartition;
namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::filter_expressions {

using silo::common::OptionalBool;

struct BoolEquals : public Expression {
  private:
   std::string column;
   OptionalBool value;

  public:
   explicit BoolEquals(std::string column, OptionalBool value);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<BoolEquals>& filter);

}  // namespace silo::query_engine::filter_expressions
