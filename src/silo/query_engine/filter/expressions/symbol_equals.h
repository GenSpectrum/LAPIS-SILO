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
class SymbolOrDot {
   std::optional<typename SymbolType::Symbol> value;

   SymbolOrDot() = default;

  public:
   static SymbolOrDot<SymbolType> dot();

   explicit SymbolOrDot(typename SymbolType::Symbol symbol);

   typename SymbolType::Symbol getSymbolOrReplaceDotWith(
      typename SymbolType::Symbol replace_dot_with
   ) const;

   [[nodiscard]] char asChar() const;
};

template <typename SymbolType>
class SymbolEquals : public Expression {
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   SymbolOrDot<SymbolType> value;

  public:
   explicit SymbolEquals(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      SymbolOrDot<SymbolType> value
   );

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

  private:
   static std::string getFilterName() {
      return fmt::format("SymbolEquals<{}>", SymbolType::SYMBOL_NAME);
   }
};

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<SymbolEquals<SymbolType>>& filter);

}  // namespace silo::query_engine::filter::expressions
