#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
class SymbolInSet : public Expression {
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::vector<typename SymbolType::Symbol> symbols;

  public:
   explicit SymbolInSet(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::vector<typename SymbolType::Symbol> symbols
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

  private:
   static std::string getFilterName() {
      return fmt::format("SymbolInSet<{}>", SymbolType::SYMBOL_NAME);
   }
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<SymbolInSet<SymbolType>>& filter);

}  // namespace silo::query_engine::filter::expressions
