#include "rhydb/query_engine/scalar_expressions/equals.h"

#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>

#include <fmt/format.h>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/filter/operators/empty.h"
#include "rhydb/query_engine/filter/operators/index_scan.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/filter/operators/selection.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/scalar_expressions/field_ref.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/query_engine/scalar_expressions/string_in_set.h"
#include "rhydb/storage/column/bool_column.h"
#include "rhydb/storage/column/date32_column.h"
#include "rhydb/storage/column/float_column.h"
#include "rhydb/storage/column/int_column.h"

using rhydb::storage::column::Date32Column;
using rhydb::storage::column::FloatColumn;
using rhydb::storage::column::Int32Column;
using rhydb::storage::column::Int64Column;

namespace rhydb::query_engine::scalar_expressions {

namespace {

/// A compilable equality has exactly one column reference (FieldRef) and one
/// value operand. Returns the (column, value) pair for that shape, or nullopt
/// when neither/both sides are column references.
struct ColumnAndValue {
   const FieldRef* column;
   const ScalarExpression* value;
};

std::optional<ColumnAndValue> splitColumnAndValue(
   const ScalarExpression* left,
   const ScalarExpression* right
) {
   const auto* left_field = dynCast<FieldRef>(left);
   const auto* right_field = dynCast<FieldRef>(right);
   if (left_field != nullptr && right_field == nullptr) {
      return ColumnAndValue{.column = left_field, .value = right};
   }
   if (right_field != nullptr && left_field == nullptr) {
      return ColumnAndValue{.column = right_field, .value = left};
   }
   return std::nullopt;
}

template <typename ColumnT, typename ValueT>
std::unique_ptr<filter::operators::Operator> compileEquals(
   const storage::Table& table,
   const std::map<std::string, ColumnT>& columns,
   const std::string& column_name,
   std::string_view type_name,
   ValueT value
) {
   CHECK_SILO_QUERY(
      columns.contains(column_name), "The column '{}' is not of type {}", column_name, type_name
   );
   const auto& column = columns.at(column_name);
   return std::make_unique<filter::operators::Selection>(
      std::make_unique<filter::operators::CompareToValueSelection<ColumnT>>(
         column, filter::operators::Comparator::EQUALS, static_cast<ColumnT::value_type>(value)
      ),
      table.row_layout
   );
}

std::unique_ptr<filter::operators::Operator> compileIntEquals(
   const storage::Table& table,
   const std::string& column_name,
   int64_t value
) {
   if (table.columns.int64_columns.contains(column_name)) {
      return compileEquals(table, table.columns.int64_columns, column_name, "int64", value);
   }
   CHECK_SILO_QUERY(
      table.columns.int32_columns.contains(column_name),
      "The column '{}' is not of type int",
      column_name
   );
   CHECK_SILO_QUERY(
      value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max(),
      "Cannot cast {} to int32. Value out of range",
      value
   );
   return compileEquals(table, table.columns.int32_columns, column_name, "int", value);
}

}  // namespace

Equals::Equals(std::unique_ptr<ScalarExpression> left, std::unique_ptr<ScalarExpression> right)
    : left(std::move(left)),
      right(std::move(right)) {}

std::string Equals::toString() const {
   return fmt::format("{} = {}", left->toString(), right->toString());
}

std::vector<schema::ColumnIdentifier> Equals::freeIUs() const {
   std::vector<schema::ColumnIdentifier> result;
   std::set<schema::ColumnIdentifier> seen;
   for (auto* operand : {left.get(), right.get()}) {
      for (auto& column : operand->freeIUs()) {
         if (seen.insert(column).second) {
            result.push_back(std::move(column));
         }
      }
   }
   return result;
}

std::unique_ptr<ScalarExpression> Equals::rewrite(
   const storage::Table& table,
   AmbiguityMode /*mode*/
) const {
   auto split = splitColumnAndValue(left.get(), right.get());

   // A non-indexed string column has no per-value index, so equality is turned
   // into a (single-element) StringInSet which knows how to scan it. Indexed
   // string columns and all other types are compiled directly by compile().
   if (split.has_value()) {
      if (const auto* string_value = dynCast<StringLiteral>(split->value)) {
         const auto& column_name = split->column->column.name;
         CHECK_SILO_QUERY(
            table.schema->getColumn(column_name).has_value(),
            "The database does not contain the column '{}'",
            column_name
         );
         CHECK_SILO_QUERY(
            table.columns.string_columns.contains(column_name) ||
               table.columns.dictionary_encoded_columns.contains(column_name),
            "The column '{}' is not of type string",
            column_name
         );
         if (table.columns.string_columns.contains(column_name)) {
            return std::make_unique<StringInSet>(
               split->column->column, std::unordered_set<std::string>{string_value->value}
            );
         }
      }
   }

   return clone();
}

std::unique_ptr<filter::operators::Operator> Equals::compile(const storage::Table& table) const {
   auto split = splitColumnAndValue(left.get(), right.get());
   CHECK_SILO_QUERY(
      split.has_value(),
      "An Equals expression can only be compiled to a filter when exactly one side is a column "
      "reference and the other a literal value"
   )
   const auto& column_name = split->column->column.name;
   const ScalarExpression* value = split->value;

   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );

   if (const auto* string_value = dynCast<StringLiteral>(value)) {
      // rewrite() validates the string column type and converts a non-indexed
      // string column into a StringInSet, so only an indexed string column can
      // reach compile() here.
      SILO_ASSERT(table.columns.dictionary_encoded_columns.contains(column_name));
      const auto& string_column = table.columns.dictionary_encoded_columns.at(column_name);
      const auto bitmap = string_column.filter(string_value->value);
      if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
         return std::make_unique<filter::operators::Empty>(table.row_layout);
      }
      return std::make_unique<filter::operators::IndexScan>(
         CopyOnWriteBitmap{bitmap.value()}, table.row_layout
      );
   }

   if (const auto* date_value = dynCast<DateLiteral>(value)) {
      return compileEquals(
         table, table.columns.date32_columns, column_name, "date", date_value->value
      );
   }

   if (const auto* int_value = dynCast<Int32Literal>(value)) {
      return compileIntEquals(table, column_name, int_value->value);
   }

   if (const auto* int_value = dynCast<Int64Literal>(value)) {
      return compileIntEquals(table, column_name, int_value->value);
   }

   if (const auto* float_value = dynCast<FloatLiteral>(value)) {
      return compileEquals(
         table, table.columns.float_columns, column_name, "float", float_value->value
      );
   }

   if (const auto* bool_value = dynCast<BoolLiteral>(value)) {
      CHECK_SILO_QUERY(
         table.columns.bool_columns.contains(column_name),
         "The column '{}' is not of type bool",
         column_name
      );
      const auto& bool_column = table.columns.bool_columns.at(column_name);
      if (bool_value->value) {
         return std::make_unique<filter::operators::IndexScan>(
            CopyOnWriteBitmap{&bool_column.true_bitmap}, table.row_layout
         );
      }
      return std::make_unique<filter::operators::IndexScan>(
         CopyOnWriteBitmap{&bool_column.false_bitmap}, table.row_layout
      );
   }

   throw IllegalQueryException(
      "An Equals expression can only be compiled to a filter when exactly one side is a column "
      "reference and the other a literal value"
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
