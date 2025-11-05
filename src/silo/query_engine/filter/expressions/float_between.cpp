#include "silo/query_engine/filter/expressions/float_between.h"

#include <cmath>
#include <memory>
#include <utility>

#include <nlohmann/json.hpp>

#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/storage/table_partition.h"

using silo::storage::column::FloatColumnPartition;

namespace silo::query_engine::filter::expressions {

// NOLINTBEGIN(bugprone-easily-swappable-parameters,readability-identifier-length)
FloatBetween::FloatBetween(
   std::string column_name,
   std::optional<double> from,
   std::optional<double> to
)
    // NOLINTEND(bugprone-easily-swappable-parameters,readability-identifier-length)
    : column_name(std::move(column_name)),
      from(from),
      to(to) {}

std::string FloatBetween::toString() const {
   const auto from_string = from.has_value() ? std::to_string(from.value()) : "unbounded";
   const auto to_string = to.has_value() ? std::to_string(to.value()) : "unbounded";

   return "[FloatBetween " + from_string + " - " + to_string + "]";
}

std::unique_ptr<Expression> FloatBetween::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FloatBetween>(column_name, from, to);
}

std::unique_ptr<silo::query_engine::filter::operators::Operator> FloatBetween::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.float_columns.contains(column_name),
      "The database does not contain the float column '{}'",
      column_name
   );
   const auto& float_column = table_partition.columns.float_columns.at(column_name);

   operators::PredicateVector predicates;
   if (from.has_value()) {
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<FloatColumnPartition>>(
            float_column, operators::Comparator::HIGHER_OR_EQUALS, from.value()
         )
      );
   }

   if (to.has_value()) {
      predicates.emplace_back(
         std::make_unique<operators::CompareToValueSelection<FloatColumnPartition>>(
            float_column, operators::Comparator::LESS, to.value()
         )
      );
   }

   if (predicates.empty()) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            CopyOnWriteBitmap{&float_column.null_bitmap}, table_partition.sequence_count
         ),
         table_partition.sequence_count
      );
   }

   return std::make_unique<operators::Selection>(
      std::move(predicates), table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FloatBetween>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a FloatBetween expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(), "The field 'column' in a FloatBetween expression must be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("from"), "The field 'from' is required in FloatBetween expression"
   );
   CHECK_SILO_QUERY(
      json["from"].is_null() || json["from"].is_number_float(),
      "The field 'from' in a FloatBetween expression must be a float or null"
   );
   CHECK_SILO_QUERY(json.contains("to"), "The field 'to' is required in a FloatBetween expression");
   CHECK_SILO_QUERY(
      json["to"].is_null() || json["to"].is_number_float(),
      "The field 'to' in a FloatBetween expression must be a float or null"
   );
   const std::string& column_name = json["column"];
   std::optional<double> value_from;
   if (json["from"].is_number_float()) {
      value_from = json["from"].get<double>();
   }
   std::optional<double> value_to;
   if (json["to"].is_number_float()) {
      value_to = json["to"].get<double>();
   }
   filter = std::make_unique<FloatBetween>(column_name, value_from, value_to);
}

}  // namespace silo::query_engine::filter::expressions
