#include "silo/query_engine/filter/expressions/string_in_set.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/expressions/string_equals.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/string_in_set.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

using storage::column::IndexedStringColumnPartition;
using storage::column::StringColumnPartition;

StringInSet::StringInSet(std::string column_name, std::unordered_set<std::string> values)
    : column_name(std::move(column_name)),
      values(std::move(values)) {}

std::string StringInSet::toString() const {
   std::vector<std::string> sorted_values;
   std::ranges::copy(values, std::back_inserter(sorted_values));
   std::ranges::sort(sorted_values);
   return fmt::format("{} IN [{}]", column_name, fmt::join(sorted_values, ","));
}

std::unique_ptr<Expression> StringInSet::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition,
   AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      table_partition.columns.string_columns.contains(column_name) ||
         table_partition.columns.indexed_string_columns.contains(column_name),
      "The database does not contain the string column '{}'",
      column_name
   );

   // We do not change expressions for StringColumn
   if (table_partition.columns.string_columns.contains(column_name)) {
      return std::make_unique<StringInSet>(column_name, values);
   }

   // We want to improve IndexedStringColumn by using our Indexes directly -> StringEquals
   std::vector<std::unique_ptr<Expression>> string_equal_expressions;
   for (const auto& value : values) {
      string_equal_expressions.emplace_back(std::make_unique<StringEquals>(column_name, value));
   }
   return std::make_unique<Or>(std::move(string_equal_expressions));
}

std::unique_ptr<operators::Operator> StringInSet::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& table_partition
) const {
   SILO_ASSERT(table_partition.columns.string_columns.contains(column_name));
   const auto& string_column = table_partition.columns.string_columns.at(column_name);
   return std::make_unique<operators::Selection>(
      std::make_unique<operators::StringInSet<StringColumnPartition>>(
         &string_column, operators::StringInSet<StringColumnPartition>::Comparator::IN, values
      ),
      table_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<StringInSet>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in a StringInSet expression"
   );
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an StringInSet expression needs to be a string"
   );
   CHECK_SILO_QUERY(
      json.contains("values"), "The field 'values' is required in a StringInSet expression"
   );
   CHECK_SILO_QUERY(
      json["values"].is_array(),
      "The field 'values' in an StringInSet expression needs to be an array"
   );
   const std::string& column_name = json["column"];
   std::unordered_set<std::string> values;
   for (const auto& value : json["values"]) {
      CHECK_SILO_QUERY(
         value.is_string(), "The field 'values' in a StringInSet may only contain strings"
      );
      values.insert(value.get<std::string>());
   }
   filter = std::make_unique<StringInSet>(column_name, values);
}

}  // namespace silo::query_engine::filter::expressions
