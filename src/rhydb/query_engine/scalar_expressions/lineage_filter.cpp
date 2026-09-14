#include "rhydb/query_engine/scalar_expressions/lineage_filter.h"

#include <cctype>
#include <optional>
#include <ranges>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "rhydb/query_engine/filter/operators/empty.h"
#include "rhydb/query_engine/filter/operators/index_scan.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/query_engine/lineage_relation_descendants.h"
#include "rhydb/query_engine/scalar_expressions/comparison.h"
#include "rhydb/query_engine/scalar_expressions/field_ref.h"
#include "rhydb/query_engine/scalar_expressions/is_null.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/query_engine/scalar_expressions/string_in_set.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::scalar_expressions {

using rhydb::common::RecombinantEdgeFollowingMode;
using rhydb::storage::column::DictionaryEncodedColumn;

LineageFilter::LineageFilter(
   schema::ColumnIdentifier column,
   std::optional<std::string> lineage,
   std::optional<RecombinantEdgeFollowingMode> sublineage_mode,
   std::string lineage_definition
)
    : column(std::move(column)),
      lineage(std::move(lineage)),
      sublineage_mode(sublineage_mode),
      lineage_definition(std::move(lineage_definition)) {}

std::string LineageFilter::toString() const {
   if (!lineage.has_value()) {
      return "NULL";
   }
   if (sublineage_mode.has_value()) {
      return "'" + lineage.value() + "*'";
   }
   return "'" + lineage.value() + "'";
}

std::vector<schema::ColumnIdentifier> LineageFilter::freeIUs() const {
   return {column};
}

std::optional<const roaring::Roaring*> LineageFilter::getBitmapForValue(
   const DictionaryEncodedColumn& lineage_column
) const {
   if (lineage == std::nullopt) {
      return lineage_column.filter(std::nullopt);
   }

   const auto value_id_opt = lineage_column.getValueId(lineage.value());

   CHECK_RHYDB_QUERY(
      value_id_opt.has_value(),
      "The lineage '{}' is not a valid lineage for column '{}'.",
      lineage.value(),
      column.name
   );

   const Idx value_id = value_id_opt.value();

   if (sublineage_mode.has_value()) {
      return lineage_column.getLineageIndex()->filterIncludingSublineages(
         value_id, sublineage_mode.value()
      );
   }
   return lineage_column.getLineageIndex()->filterExcludingSublineages(value_id);
}

namespace {

/// True when the column carries the in-memory lineage index, which resolves the filter through a
/// precomputed bitmap instead of the relation table.
bool hasLineageIndex(const storage::Table& table, const schema::ColumnIdentifier& column) {
   const auto found = table.columns.dictionary_encoded_columns.find(column.name);
   return found != table.columns.dictionary_encoded_columns.end() &&
          found->second.getLineageIndex().has_value();
}

}  // namespace

std::unique_ptr<ScalarExpression> LineageFilter::rewrite(
   const storage::Table& table,
   AmbiguityMode mode
) const {
   CHECK_RHYDB_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   if (hasLineageIndex(table, column)) {
      return std::make_unique<LineageFilter>(column, lineage, sublineage_mode, lineage_definition);
   }

   // The replacements are themselves rewritten before being handed back: a rewrite is not applied
   // to its own result, and both of them have a rewrite of their own that matters here (a
   // StringInSet over a dictionary-encoded column becomes a union of index lookups).
   if (!lineage.has_value()) {
      return IsNull{column}.rewrite(table, mode);
   }

   const auto definition = table.lineage_definitions.find(lineage_definition);
   CHECK_RHYDB_QUERY(
      definition != table.lineage_definitions.end(),
      "'{}' is not a lineage definition of this database.",
      lineage_definition
   );

   // A query may name a lineage by any of its aliases, so the alias table - when the definition
   // declares one - has the first say.
   const std::string target =
      definition->second.aliases == nullptr
         ? lineage.value()
         : resolveLineageAlias(*definition->second.aliases, lineage.value());

   if (!sublineage_mode.has_value()) {
      // An exact match needs no hierarchy: equality on the column does it.
      return Comparison{
         std::make_unique<FieldRef>(column),
         std::make_unique<StringLiteral>(target),
         filter::operators::Comparator::EQUALS
      }
         .rewrite(table, mode);
   }

   auto sublineages =
      computeLineageWithSublineages(*definition->second.relation, target, sublineage_mode.value());
   CHECK_RHYDB_QUERY(
      sublineages.has_value(),
      "The lineage '{}' is not a valid lineage for column '{}'.",
      lineage.value(),
      column.name
   );
   return StringInSet{column, std::move(sublineages.value())}.rewrite(table, mode);
}

std::unique_ptr<filter::operators::Operator> LineageFilter::compile(const storage::Table& table
) const {
   CHECK_RHYDB_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
   CHECK_RHYDB_QUERY(
      table.columns.dictionary_encoded_columns.contains(column.name) &&
         table.columns.dictionary_encoded_columns.at(column.name).getLineageIndex().has_value(),
      "The database does not contain a lineage index for the column '{}'",
      column.name
   );

   const auto& lineage_column = table.columns.dictionary_encoded_columns.at(column.name);
   std::optional<const roaring::Roaring*> bitmap = getBitmapForValue(lineage_column);

   if (bitmap == std::nullopt) {
      return std::make_unique<filter::operators::Empty>(table.row_layout);
   }
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{bitmap.value()}, table.row_layout
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
