#include "silo/query_engine/filter/expressions/true.h"

#include <string>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

True::True() = default;

std::string True::toString() const {
   return "True";
}

std::unique_ptr<Expression> True::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   Expression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<True>();
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> True::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   return std::make_unique<operators::Full>(table_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& /*json*/, std::unique_ptr<True>& filter) {
   filter = std::make_unique<True>();
}

}  // namespace silo::query_engine::filter::expressions
