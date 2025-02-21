#include "silo/query_engine/filter_expressions/maybe.h"

#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

Maybe::Maybe(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Maybe::toString() const {
   return "Maybe (" + child->toString() + ")";
}
std::unique_ptr<silo::query_engine::operators::Operator> Maybe::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   silo::query_engine::filter_expressions::Expression::AmbiguityMode /*mode*/
) const {
   return child->compile(database, database_partition, AmbiguityMode::UPPER_BOUND);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Maybe>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Maybe expression");
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Maybe>(std::move(child));
}

}  // namespace silo::query_engine::filter_expressions
