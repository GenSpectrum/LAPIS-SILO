#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine::operators {
class Operator;
}
namespace silo {
class Database;
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

class Expression {
  protected:
   Expression();

  public:
   virtual ~Expression() = default;

   /// UPPER_BOUND returns the upper bound of sequences matching this expression (i.e. ambiguous
   /// codes count as matches), LOWER_BOUND returns the lower bound of sequences matching this
   /// expression (i.e. ambiguous codes in negations count as matches)
   /// NONE does not specially consider ambiguous symbols
   enum AmbiguityMode { UPPER_BOUND, LOWER_BOUND, NONE };

   virtual std::string toString() const = 0;

   [[nodiscard]] virtual std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const = 0;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Expression>& filter);

Expression::AmbiguityMode invertMode(Expression::AmbiguityMode mode);

}  // namespace silo::query_engine::filter_expressions
