#include "silo/query_engine/filter_expressions/false.h"

#include <string>

#include "silo/database.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

False::False() = default;

std::string False::toString() const {
   return "False";
}

std::unique_ptr<silo::query_engine::operators::Operator> False::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<operators::Empty>(database_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& /*json*/, std::unique_ptr<False>& filter) {
   filter = std::make_unique<False>();
}

}  // namespace silo::query_engine::filter_expressions