#include "silo/query_engine/filter/expressions/int_between.h"

#include <memory>
#include <utility>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::IntColumnPartition;

namespace silo::query_engine::filter::expressions {

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-identifier-length)
IntBetween::IntBetween(
   std::string column_name,
   std::optional<uint32_t> from,
   std::optional<uint32_t> to
)
    // NOLINTEND(bugprone-easily-swappable-parameters,readability-identifier-length)
    : column_name(std::move(column_name)),
      from(from),
      to(to) {}

std::string IntBetween::toString() const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[IntBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<Expression> IntBetween::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<IntBetween>(column_name, from, to);
}

std::unique_ptr<operators::Operator> IntBetween::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.int_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   const auto& int_column = table_partition.columns.int_columns.at(column_name);

   operators::PredicateVector predicates;
   if (from.has_value()) {
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<IntColumnPartition>>(
            int_column, operators::Comparator::HIGHER_OR_EQUALS, from.value()
         )
      );
   }
   if (to.has_value()) {
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<IntColumnPartition>>(
            int_column, operators::Comparator::LESS_OR_EQUALS, to.value()
         )
      );
   }

   if (predicates.empty()) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            CopyOnWriteBitmap{&int_column.null_bitmap}, table_partition.sequence_count
         ),
         table_partition.sequence_count
      );
   }

   auto result =
      std::make_unique<operators::Selection>(std::move(predicates), table_partition.sequence_count);

   SPDLOG_TRACE("Compiled IntBetween filter expression to {}", result->toString());

   return std::move(result);
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<IntBetween>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a IntBetween expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a IntBetween expression must be a string"
   );
   CHECK_SILO_QUERY(json.contains("from"), "The field 'from' is required in IntBetween expression");
   CHECK_SILO_QUERY(
      json["from"].is_number_integer() || json["from"].is_null(),
      "The field 'from' in an IntBetween expression must be an integer in [-2147483648; "
      "2147483647] or null"
   );
   CHECK_SILO_QUERY(json.contains("to"), "The field 'to' is required in a IntBetween expression");
   CHECK_SILO_QUERY(
      (json["to"].is_number_integer() && json["to"].is_number_integer()) || json["to"].is_null(),
      "The field 'to' in an IntBetween expression must be an integer in [-2147483648; 2147483647] "
      "or null"
   );
   const std::string& column_name = json["column"];
   std::optional<int32_t> value_from;
   if (json["from"].is_number_integer()) {
      value_from = json["from"].get<int32_t>();
   }
   std::optional<int32_t> value_to;
   if (json["to"].is_number_integer()) {
      value_to = json["to"].get<int32_t>();
   }
   filter = std::make_unique<IntBetween>(column_name, value_from, value_to);
}

}  // namespace silo::query_engine::filter::expressions
