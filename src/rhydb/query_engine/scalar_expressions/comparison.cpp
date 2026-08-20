#include "rhydb/query_engine/scalar_expressions/comparison.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <roaring/roaring.hh>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/filter/operators/empty.h"
#include "rhydb/query_engine/filter/operators/index_scan.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/filter/operators/selection.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/scalar_expressions/field_ref.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/storage/column/date32_column.h"
#include "rhydb/storage/column/float_column.h"
#include "rhydb/storage/column/int_column.h"
#include "rhydb/storage/column/string_column.h"

using rhydb::query_engine::filter::operators::Comparator;
using rhydb::storage::column::Date32Column;
using rhydb::storage::column::FloatColumn;
using rhydb::storage::column::Int32Column;
using rhydb::storage::column::Int64Column;
using rhydb::storage::column::StringColumn;

namespace rhydb::query_engine::scalar_expressions {

namespace {

/// A compilable comparison has exactly one column reference (FieldRef) and one
/// value operand. Returns the (column, value) pair for that shape together with
/// whether the column was on the right-hand side (so compile() can flip the
/// comparator), or nullopt when neither/both sides are column references.
struct ColumnAndValue {
   const FieldRef* column;
   const ScalarExpression* value;
   bool column_on_right;
};

std::optional<ColumnAndValue> splitColumnAndValue(
   const ScalarExpression* left,
   const ScalarExpression* right
) {
   const auto* left_field = dynCast<FieldRef>(left);
   const auto* right_field = dynCast<FieldRef>(right);
   if (left_field != nullptr && right_field == nullptr) {
      return ColumnAndValue{.column = left_field, .value = right, .column_on_right = false};
   }
   if (right_field != nullptr && left_field == nullptr) {
      return ColumnAndValue{.column = right_field, .value = left, .column_on_right = true};
   }
   return std::nullopt;
}

/// Inverts an ordering comparator so that `value <op> column` becomes the
/// equivalent `column <flipped> value`. Equality/inequality are unaffected.
Comparator flipComparator(Comparator comparator) {
   switch (comparator) {
      case Comparator::LESS:
         return Comparator::HIGHER;
      case Comparator::HIGHER:
         return Comparator::LESS;
      case Comparator::LESS_OR_EQUALS:
         return Comparator::HIGHER_OR_EQUALS;
      case Comparator::HIGHER_OR_EQUALS:
         return Comparator::LESS_OR_EQUALS;
      case Comparator::EQUALS:
      case Comparator::NOT_EQUALS:
         return comparator;
   }
   SILO_UNREACHABLE();
}

/// Lexicographic evaluation of `actual <op> literal` for the dictionary fast path.
bool matchesComparator(std::string_view actual, Comparator comparator, std::string_view literal) {
   switch (comparator) {
      case Comparator::EQUALS:
         return actual == literal;
      case Comparator::NOT_EQUALS:
         return actual != literal;
      case Comparator::LESS:
         return actual < literal;
      case Comparator::HIGHER:
         return actual > literal;
      case Comparator::LESS_OR_EQUALS:
         return actual <= literal;
      case Comparator::HIGHER_OR_EQUALS:
         return actual >= literal;
   }
   SILO_UNREACHABLE();
}

template <typename ColumnType, typename ColumnMap>
std::unique_ptr<filter::operators::Operator> compileTypedComparison(
   const storage::Table& table,
   const ColumnMap& column_map,
   const std::string& column_name,
   Comparator comparator,
   std::string_view type_name,
   typename ColumnType::value_type value
) {
   CHECK_SILO_QUERY(
      column_map.contains(column_name), "The column '{}' is not of type {}", column_name, type_name
   );
   return std::make_unique<filter::operators::Selection>(
      std::make_unique<filter::operators::CompareToValueSelection<ColumnType>>(
         column_map.at(column_name), comparator, value
      ),
      table.row_layout
   );
}

std::unique_ptr<filter::operators::Operator> compileStringComparison(
   const storage::Table& table,
   const std::string& column_name,
   Comparator comparator,
   const std::string& literal
) {
   if (table.columns.string_columns.contains(column_name)) {
      const auto& string_column = table.columns.string_columns.at(column_name);
      return std::make_unique<filter::operators::Selection>(
         std::make_unique<filter::operators::CompareToValueSelection<StringColumn>>(
            string_column, comparator, literal
         ),
         table.row_layout
      );
   }

   CHECK_SILO_QUERY(
      table.columns.dictionary_encoded_columns.contains(column_name),
      "The column '{}' is not of type string",
      column_name
   );

   // Bitmap-union fast path: every distinct dictionary value whose string matches
   // the comparator contributes its row bitmap. Null rows are naturally excluded
   // because the indexed bitmaps are disjoint from the null bitmap.
   const auto& dictionary_column = table.columns.dictionary_encoded_columns.at(column_name);
   roaring::Roaring unioned;
   for (const auto& [dict_id, bitmap] : dictionary_column.getIndexedValues()) {
      if (matchesComparator(dictionary_column.lookupValue(dict_id), comparator, literal)) {
         unioned |= bitmap;
      }
   }
   if (unioned.isEmpty()) {
      return std::make_unique<filter::operators::Empty>(table.row_layout);
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{std::move(unioned)}, table.row_layout
   );
}

/// Integer literals are width-agnostic (kept as int64); routing to the actual
/// int32/int64 column and the int32 range check happen here, once the column
/// type is known. Mirrors Equals' compileIntEquals.
std::unique_ptr<filter::operators::Operator> compileIntComparison(
   const storage::Table& table,
   const std::string& column_name,
   Comparator comparator,
   int64_t value
) {
   if (table.columns.int64_columns.contains(column_name)) {
      return compileTypedComparison<Int64Column>(
         table, table.columns.int64_columns, column_name, comparator, "int64", value
      );
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
   return compileTypedComparison<Int32Column>(
      table,
      table.columns.int32_columns,
      column_name,
      comparator,
      "int",
      static_cast<int32_t>(value)
   );
}

}  // namespace

Comparison::Comparison(
   std::unique_ptr<ScalarExpression> left,
   std::unique_ptr<ScalarExpression> right,
   Comparator comparator
)
    : left(std::move(left)),
      right(std::move(right)),
      comparator(comparator) {}

std::string Comparison::toString() const {
   return fmt::format(
      "{} {} {}",
      left->toString(),
      filter::operators::displayComparator(comparator),
      right->toString()
   );
}

std::vector<schema::ColumnIdentifier> Comparison::freeIUs() const {
   std::vector<schema::ColumnIdentifier> result = left->freeIUs();
   std::ranges::move(right->freeIUs(), std::back_inserter(result));
   return result;
}

std::unique_ptr<ScalarExpression> Comparison::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return clone();
}

std::unique_ptr<filter::operators::Operator> Comparison::compile(const storage::Table& table
) const {
   auto split = splitColumnAndValue(left.get(), right.get());
   CHECK_SILO_QUERY(
      split.has_value(),
      "A Comparison expression can only be compiled to a filter when exactly one side is a column "
      "reference and the other a literal value"
   );
   const auto& column_name = split->column->column.name;
   const ScalarExpression* value = split->value;
   // If the column sits on the right (`literal <op> column`) the comparator must
   // be inverted so that the column-relative form is evaluated correctly.
   const Comparator effective_comparator =
      split->column_on_right ? flipComparator(comparator) : comparator;

   CHECK_SILO_QUERY(
      table.schema->getColumn(column_name).has_value(),
      "The database does not contain the column '{}'",
      column_name
   );

   if (const auto* int_value = dynCast<Int32Literal>(value)) {
      return compileIntComparison(table, column_name, effective_comparator, int_value->value);
   }

   if (const auto* int_value = dynCast<Int64Literal>(value)) {
      return compileIntComparison(table, column_name, effective_comparator, int_value->value);
   }

   if (const auto* float_value = dynCast<FloatLiteral>(value)) {
      return compileTypedComparison<FloatColumn>(
         table,
         table.columns.float_columns,
         column_name,
         effective_comparator,
         "float",
         float_value->value
      );
   }

   if (const auto* date_value = dynCast<DateLiteral>(value)) {
      return compileTypedComparison<Date32Column>(
         table,
         table.columns.date32_columns,
         column_name,
         effective_comparator,
         "date",
         date_value->value
      );
   }

   if (const auto* string_value = dynCast<StringLiteral>(value)) {
      return compileStringComparison(table, column_name, effective_comparator, string_value->value);
   }

   if (dynCast<BoolLiteral>(value) != nullptr) {
      CHECK_SILO_QUERY(
         false,
         "The comparison operators <,>,<=,>= are not supported for boolean column '{}'",
         column_name
      );
   }

   throw IllegalQueryException(
      "A Comparison expression can only be compiled to a filter when exactly one side is a column "
      "reference and the other a literal value"
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
