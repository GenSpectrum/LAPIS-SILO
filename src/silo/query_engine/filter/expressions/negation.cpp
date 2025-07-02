#include "silo/query_engine/filter/expressions/negation.h"

#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

Negation::Negation(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Negation::toString() const {
   return "!(" + child->toString() + ")";
}

std::unique_ptr<operators::Operator> Negation::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
) const {
   auto child_operator = child->compile(table, table_partition, invertMode(mode));
   return operators::Operator::negate(std::move(child_operator));
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Negation>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Not expression");
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Negation>(std::move(child));
}

}  // namespace silo::query_engine::filter::expressions
