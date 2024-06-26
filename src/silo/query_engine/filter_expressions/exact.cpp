#include "silo/query_engine/filter_expressions/exact.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo {
class Database;
class DatabasePartition;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

Exact::Exact(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Exact::toString() const {
   return fmt::format("Exact ({})", child->toString());
}
std::unique_ptr<silo::query_engine::operators::Operator> Exact::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter_expressions::Expression::AmbiguityMode /*mode*/
) const {
   return child->compile(database, database_partition, AmbiguityMode::LOWER_BOUND);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Exact>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Exact expression");
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Exact>(std::move(child));
}

}  // namespace silo::query_engine::filter_expressions
