#include "silo/query_engine/filter_expressions/true.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/full.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

True::True() = default;

std::string True::toString(const silo::Database& /*database*/) {
   return "True";
}

std::unique_ptr<silo::query_engine::operators::Operator> True::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<operators::Full>(database_partition.sequenceCount);
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<True>& filter) {
   filter = std::make_unique<True>();
}

}  // namespace silo::query_engine::filter_expressions