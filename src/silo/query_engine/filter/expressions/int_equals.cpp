#include "silo/query_engine/filter/expressions/int_equals.h"

#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::IntColumnPartition;

namespace silo::query_engine::filter::expressions {

IntEquals::IntEquals(std::string column_name, std::optional<uint32_t> value)
    : column_name(std::move(column_name)),
      value(value) {}

std::string IntEquals::toString() const {
   if (value.has_value()) {
      return fmt::format("{} = '{}'", column_name, value.value());
   }
   return fmt::format("{} IS NULL", column_name);
}

std::unique_ptr<Expression> IntEquals::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<IntEquals>(column_name, value);
}

std::unique_ptr<operators::Operator> IntEquals::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.int_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& int_column = table_partition.columns.int_columns.at(column_name);

   if (value.has_value()) {
      return std::make_unique<operators::Selection>(
         std::make_unique<operators::CompareToValueSelection<IntColumnPartition>>(
            int_column, operators::Comparator::EQUALS, value.value()
         ),
         table_partition.sequence_count
      );
   }
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{&int_column.null_bitmap}, table_partition.sequence_count
   );
}

}  // namespace silo::query_engine::filter::expressions
