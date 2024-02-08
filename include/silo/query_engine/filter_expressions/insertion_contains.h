#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class Database;
class DatabasePartition;
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
}  // namespace silo

namespace silo::query_engine::filter_expressions {

template <typename SymbolType>
struct InsertionContains : public Expression {
  private:
   std::vector<std::string> column_names;
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::string value;

  public:
   explicit InsertionContains(
      std::vector<std::string>&& column_names,
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::string value
   );

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains<SymbolType>>& filter);

}  // namespace silo::query_engine::filter_expressions
