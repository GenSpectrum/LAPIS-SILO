#include "rhydb/query_engine/scalar_expressions/comparison.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
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
#include "rhydb/query_engine/scalar_expressions/string_in_set.h"
#include "rhydb/storage/column/bool_column.h"
#include "rhydb/storage/column/date32_column.h"
#include "rhydb/storage/column/float_column.h"
#include "rhydb/storage/column/int_column.h"
#include "rhydb/storage/column/string_column.h"

using rhydb::query_engine::filter::operators::Comparator;
using rhydb::query_engine::filter::operators::CompareToValueSelection;
using rhydb::query_engine::filter::operators::displayComparator;
using rhydb::query_engine::filter::operators::Empty;
using rhydb::query_engine::filter::operators::IndexScan;
using rhydb::query_engine::filter::operators::Operator;
using rhydb::query_engine::filter::operators::Selection;
using rhydb::schema::ColumnIdentifier;
using rhydb::storage::Table;
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
std::unique_ptr<Operator> compileTypedComparison(
   const Table& table,
   const ColumnMap& column_map,
   const std::string& column_name,
   Comparator comparator,
   std::string_view type_name,
   typename ColumnType::value_type value
) {
   CHECK_SILO_QUERY(
      column_map.contains(column_name), "The column '{}' is not of type {}", column_name, type_name
   );
   return std::make_unique<Selection>(
      std::make_unique<CompareToValueSelection<ColumnType>>(
         column_map.at(column_name), comparator, value
      ),
      table.row_layout
   );
}

std::unique_ptr<Operator> compileStringComparison(
   const Table& table,
   const std::string& column_name,
   Comparator comparator,
   const std::string& literal
) {
   if (table.columns.string_columns.contains(column_name)) {
      // Equality on non-indexed string columns is always converted to StringInSet
      SILO_ASSERT(comparator != Comparator::EQUALS);
      const auto& string_column = table.columns.string_columns.at(column_name);
      return std::make_unique<Selection>(
         std::make_unique<CompareToValueSelection<StringColumn>>(
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

   const auto& dictionary_column = table.columns.dictionary_encoded_columns.at(column_name);

   if (comparator == Comparator::EQUALS) {
      const auto bitmap = dictionary_column.filter(literal);
      if (bitmap == std::nullopt || bitmap.value()->isEmpty()) {
         return std::make_unique<Empty>(table.row_layout);
      }
      return std::make_unique<IndexScan>(CopyOnWriteBitmap{bitmap.value()}, table.row_layout);
   }

   // Inequality is the complement of one index lookup: every row except those holding
   // the literal and except the null rows (which never match a comparison). Going
   // through the complement keeps this a single lookup instead of unioning the bitmap
   // of every other distinct value, which matters on high-cardinality columns.
   if (comparator == Comparator::NOT_EQUALS) {
      roaring::Roaring excluded = dictionary_column.null_bitmap;
      if (const auto bitmap = dictionary_column.filter(literal); bitmap != std::nullopt) {
         excluded |= *bitmap.value();
      }
      // `excluded` is a subset of the valid row universe, so covering every row means the
      // complement is empty. Return Empty directly, mirroring the equality path, instead of
      // a Complement that evaluates to nothing.
      if (excluded.cardinality() == table.row_layout.numRows()) {
         return std::make_unique<Empty>(table.row_layout);
      }
      return Operator::negate(
         std::make_unique<IndexScan>(CopyOnWriteBitmap{std::move(excluded)}, table.row_layout)
      );
   }

   // Bitmap-union fast path: every distinct dictionary value whose string matches
   // the comparator contributes its row bitmap. Null rows are naturally excluded
   // because the indexed bitmaps are disjoint from the null bitmap.
   roaring::Roaring unioned;
   for (const auto& [dict_id, bitmap] : dictionary_column.getIndexedValues()) {
      if (matchesComparator(dictionary_column.lookupValue(dict_id), comparator, literal)) {
         unioned |= bitmap;
      }
   }
   if (unioned.isEmpty()) {
      return std::make_unique<Empty>(table.row_layout);
   }
   return std::make_unique<IndexScan>(CopyOnWriteBitmap{std::move(unioned)}, table.row_layout);
}

/// Boolean columns keep a bitmap per truth value, so (in)equality is a plain index
/// scan. Ordering comparisons are not defined for booleans and are rejected.
std::unique_ptr<Operator> compileBoolComparison(
   const Table& table,
   const std::string& column_name,
   Comparator comparator,
   bool value
) {
   // The column type is checked first so that comparing a non-bool column against a
   // bool literal reports the type mismatch rather than claiming the column is boolean.
   CHECK_SILO_QUERY(
      table.columns.bool_columns.contains(column_name),
      "The column '{}' is not of type bool",
      column_name
   );
   CHECK_SILO_QUERY(
      comparator == Comparator::EQUALS || comparator == Comparator::NOT_EQUALS,
      "The comparison operators <,>,<=,>= are not supported for boolean column '{}'",
      column_name
   );
   const auto& bool_column = table.columns.bool_columns.at(column_name);
   // `column <> true` selects the same rows as `column = false`. Null rows are in
   // neither bitmap, so they are excluded either way, consistent with the other
   // comparison operators.
   const bool select_true_bitmap = (comparator == Comparator::EQUALS) == value;
   return std::make_unique<IndexScan>(
      CopyOnWriteBitmap{select_true_bitmap ? &bool_column.true_bitmap : &bool_column.false_bitmap},
      table.row_layout
   );
}

/// Integer literals are width-agnostic (kept as int64); routing to the actual
/// int32/int64 column and the int32 range check happen here, once the column
/// type is known.
std::unique_ptr<Operator> compileIntComparison(
   const Table& table,
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
      "{} {} {}", left->toString(), displayComparator(comparator), right->toString()
   );
}

std::vector<ColumnIdentifier> Comparison::freeIUs() const {
   std::vector<ColumnIdentifier> result = left->freeIUs();
   std::ranges::move(right->freeIUs(), std::back_inserter(result));
   return result;
}

std::unique_ptr<ScalarExpression> Comparison::rewrite(
   const Table& table,
   AmbiguityMode /*mode*/
) const {
   // Only equality is rewritten. A non-indexed string column has no per-value index,
   // so `column = 'value'` becomes a (single-element) StringInSet, which knows how to
   // scan it and which Or can merge with sibling StringInSets over the same column.
   // Dictionary-encoded columns and all other types are compiled directly.
   if (comparator != Comparator::EQUALS) {
      return clone();
   }

   auto split = splitColumnAndValue(left.get(), right.get());
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

std::unique_ptr<Operator> Comparison::compile(const Table& table) const {
   auto split = splitColumnAndValue(left.get(), right.get());
   CHECK_SILO_QUERY(
      split.has_value(),
      "A Comparison expression can only be compiled to a filter when exactly one side is a column "
      "reference and the other a literal value"
   );
   const auto& column_name = split->column->column.name;
   const ScalarExpression* value = split->value;
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

   if (const auto* bool_value = dynCast<BoolLiteral>(value)) {
      return compileBoolComparison(table, column_name, effective_comparator, bool_value->value);
   }

   throw IllegalQueryException(
      "Unsupported value type in comparison with column '{}': the value must be an int, float, "
      "date, string, or bool literal",
      column_name
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
