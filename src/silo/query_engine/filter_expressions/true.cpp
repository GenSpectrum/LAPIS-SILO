#include "silo/query_engine/filter_expressions/true.h"

#include <string>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/full.h"
#include "silo/storage/database_partition.h"

namespace silo {
namespace query_engine::operators {
class Operator;
}  // namespace query_engine::operators
struct Database;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

True::True() = default;

std::string True::toString() const {
   return "True";
}

std::unique_ptr<silo::query_engine::operators::Operator> True::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<operators::Full>(database_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& /*json*/, std::unique_ptr<True>& filter) {
   filter = std::make_unique<True>();
}

}  // namespace silo::query_engine::filter_expressions