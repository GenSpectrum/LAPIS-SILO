#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

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

struct HasMutation : public Expression {
  private:
   std::optional<std::string> nuc_sequence_name;
   uint32_t position;

  public:
   explicit HasMutation(std::optional<std::string> nuc_sequence_name, uint32_t position);

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<HasMutation>& filter);

}  // namespace silo::query_engine::filter_expressions
