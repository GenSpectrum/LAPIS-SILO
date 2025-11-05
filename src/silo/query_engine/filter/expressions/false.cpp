#include "silo/query_engine/filter/expressions/false.h"

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

False::False() = default;

std::string False::toString() const {
   return "False";
}

std::unique_ptr<silo::query_engine::filter::expressions::Expression> False::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<False>();
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> False::compile(
   const storage::Table& /*table*/,
   const silo::storage::TablePartition& table_partition
) const {
   return std::make_unique<operators::Empty>(table_partition.sequence_count);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& /*json*/, std::unique_ptr<False>& filter) {
   filter = std::make_unique<False>();
}

}  // namespace silo::query_engine::filter::expressions