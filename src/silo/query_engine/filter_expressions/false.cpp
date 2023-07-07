#include "silo/query_engine/filter_expressions/false.h"

#include <string>

#include "silo/query_engine/operators/empty.h"
#include "silo/storage/database_partition.h"

namespace silo {
namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
struct Database;
}  // namespace silo

namespace silo::query_engine::filter_expressions {

False::False() = default;

std::string False::toString(const silo::Database& /*database*/) const {
   return "False";
}

std::unique_ptr<silo::query_engine::operators::Operator> False::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<operators::Empty>(database_partition.sequenceCount);
}

void from_json(const nlohmann::json& /*json*/, std::unique_ptr<False>& filter) {
   filter = std::make_unique<False>();
}

}  // namespace silo::query_engine::filter_expressions