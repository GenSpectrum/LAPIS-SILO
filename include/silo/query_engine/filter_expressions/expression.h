#ifndef SILO_EXPRESSION_H
#define SILO_EXPRESSION_H

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine::operators {
struct Operator;
}
namespace silo {
struct Database;
struct DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

struct Expression {
   enum AmbiguityMode { UPPER_BOUND, LOWER_BOUND, NONE };

   Expression();
   virtual ~Expression() = default;

   virtual std::string toString(const silo::Database& database) const = 0;

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

#endif  // SILO_EXPRESSION_H
