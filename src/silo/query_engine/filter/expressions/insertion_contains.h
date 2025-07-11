#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
class InsertionContains : public Expression {
  private:
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::string value;

  public:
   explicit InsertionContains(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::string value
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains<SymbolType>>& filter);

}  // namespace silo::query_engine::filter::expressions
