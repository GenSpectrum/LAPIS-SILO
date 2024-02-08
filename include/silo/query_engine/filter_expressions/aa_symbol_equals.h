#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo {
class Database;
class DatabasePartition;

namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
}  // namespace silo

namespace silo::query_engine::filter_expressions {

struct AASymbolEquals : public Expression {
   std::string aa_sequence_name;
   uint32_t position_idx;
   std::optional<AminoAcid::Symbol> value;

   explicit AASymbolEquals(
      std::string aa_sequence_name,
      uint32_t position_idx,
      std::optional<AminoAcid::Symbol> value
   );

   [[nodiscard]] std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AASymbolEquals>& filter);

}  // namespace silo::query_engine::filter_expressions
