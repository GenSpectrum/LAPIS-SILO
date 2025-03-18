#include "silo/query_engine/filter/expressions/exact.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

Exact::Exact(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Exact::toString() const {
   return fmt::format("Exact ({})", child->toString());
}
std::unique_ptr<silo::query_engine::filter::operators::Operator> Exact::compile(
   const Database& database,
   const storage::TablePartition& database_partition,
   silo::query_engine::filter::expressions::Expression::AmbiguityMode /*mode*/
) const {
   return child->compile(database, database_partition, AmbiguityMode::LOWER_BOUND);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Exact>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Exact expression");
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Exact>(std::move(child));
}

}  // namespace silo::query_engine::filter::expressions
