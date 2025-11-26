#include "silo/query_engine/filter/expressions/exact.h"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

Exact::Exact(std::unique_ptr<Expression> child)
    : child(std::move(child)) {}

std::string Exact::toString() const {
   return fmt::format("Exact ({})", child->toString());
}

std::unique_ptr<Expression> Exact::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   return child->rewrite(table, table_partition, AmbiguityMode::LOWER_BOUND);
}

std::unique_ptr<operators::Operator> Exact::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/
) const {
   throw QueryCompilationException{"Exact expression must be elimitated in query rewrite phase"};
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Exact>& filter) {
   CHECK_SILO_QUERY(json.contains("child"), "The field 'child' is required in a Exact expression");
   auto child = json["child"].get<std::unique_ptr<Expression>>();
   filter = std::make_unique<Exact>(std::move(child));
}

}  // namespace silo::query_engine::filter::expressions
