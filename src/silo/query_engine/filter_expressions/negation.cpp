#include "silo/query_engine/filter_expressions/negation.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

Negation::Negation(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Negation::toString(const silo::Database& database) {
   return "!(" + child->toString(database) + ")";
}

std::unique_ptr<operators::Operator> Negation::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode mode
) const {
   auto child_operator = child->compile(database, database_partition, invertMode(mode));
   return child_operator->negate();
}

void from_json(const nlohmann::json& json, std::unique_ptr<Negation>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Not expression")
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Negation>(std::move(child));
}

}  // namespace silo::query_engine::filter_expressions