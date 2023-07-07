#ifndef SILO_AA_SYMBOL_EQUALS_H
#define SILO_AA_SYMBOL_EQUALS_H

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

struct AASymbolEquals : public Expression {
   std::string aa_sequence_name;
   uint32_t position;
   char value;

   explicit AASymbolEquals(std::string aa_sequence_name, uint32_t position, char value);

   std::string toString(const Database& database) const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AASymbolEquals>& filter);

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_AA_SYMBOL_EQUALS_H
