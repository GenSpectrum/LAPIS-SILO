#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo::query_engine::operators {
struct Operator;
}
namespace silo {
struct Database;
struct DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

struct PositionalFilter {
   std::optional<std::string> sequence_name;
   uint32_t position;
   std::variant<Nucleotide::Symbol, AminoAcid::Symbol> symbol;
};

struct Expression {
   /// UPPER_BOUND returns the upper bound of sequences matching this expression (i.e. ambiguous
   /// codes count as matches), LOWER_BOUND returns the lower bound of sequences matching this
   /// expression (i.e. ambiguous codes in negations count as matches)
   /// NONE does not specially consider ambiguous symbols
   enum AmbiguityMode { UPPER_BOUND, LOWER_BOUND, NONE };

   Expression();
   virtual ~Expression() = default;

   virtual std::string toString() const = 0;

   [[nodiscard]] virtual std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const = 0;

   static std::optional<PositionalFilter> isPositionalFilterForSymbol(
      const std::unique_ptr<Expression>& expression
   );

   static std::optional<PositionalFilter> isNegatedPositionalFilterForSymbol(
      const std::unique_ptr<Expression>& expression
   );
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Expression>& filter);

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode);

}  // namespace silo::query_engine::filter_expressions
