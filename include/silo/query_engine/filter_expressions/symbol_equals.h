#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
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

template <typename SymbolType>
struct SymbolEquals : public Expression {
   std::optional<std::string> sequence_name;
   uint32_t position_idx;
   std::optional<typename SymbolType::Symbol> value;

   explicit SymbolEquals(
      std::optional<std::string> sequence_name,
      uint32_t position_idx,
      std::optional<typename SymbolType::Symbol> value
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
void from_json(const nlohmann::json& json, std::unique_ptr<SymbolEquals<SymbolType>>& filter);

}  // namespace silo::query_engine::filter_expressions
