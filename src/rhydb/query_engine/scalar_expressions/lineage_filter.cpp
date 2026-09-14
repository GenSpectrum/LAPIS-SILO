#include "rhydb/query_engine/scalar_expressions/lineage_filter.h"

#include <optional>
#include <utility>

#include <fmt/format.h>

#include "rhydb/initialize/lineage_relation_table.h"
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

std::unique_ptr<ScalarExpression> LineageFilter::rewrite(
   const storage::Table& table,
   AmbiguityMode mode
) const {
   CHECK_RHYDB_QUERY(
      table.schema->getColumn(column.name).has_value(),
      "The database does not contain the column '{}'",
      column.name
   );
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

std::unique_ptr<filter::operators::Operator> LineageFilter::compile(const storage::Table& /*table*/
) const {
   throw IllegalQueryException(
      "internal error: lineage(...) on column '{}' reached compile() without being rewritten",
      column.name
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
