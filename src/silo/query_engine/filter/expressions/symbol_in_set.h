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

class Or;

template <typename SymbolType>
class SymbolInSet : public Expression {
   friend class Or;  // For optimization under Or

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

   [[nodiscard]] const std::optional<std::string>& getSequenceName() const { return sequence_name; }
   [[nodiscard]] uint32_t getPositionIdx() const { return position_idx; }
   [[nodiscard]] const std::vector<typename SymbolType::Symbol>& getSymbols() const {
      return symbols;
   }

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
      return fmt::format("SymbolInSet<{}>", SymbolType::SYMBOL_NAME);
   }
};

}  // namespace silo::query_engine::filter::expressions
