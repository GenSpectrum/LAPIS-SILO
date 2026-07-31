#include "silo/query_engine/scalar_expressions/is_null.h"

#include <utility>

#include <fmt/format.h>

#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::scalar_expressions {

IsNull::IsNull(schema::ColumnIdentifier column)
    : column(std::move(column)) {}

std::string IsNull::toString() const {
   return fmt::format("{} IS NULL", column.name);
}

std::vector<schema::ColumnIdentifier> IsNull::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> IsNull::rewrite(
   const storage::Table& /*table*/,
   ScalarExpression::AmbiguityMode /*mode*/
) const {
   return std::make_unique<IsNull>(column);
}

std::unique_ptr<filter::operators::Operator> IsNull::compile(const storage::Table& table) const {
   const auto& maybe_target_column = table.schema->getColumn(column.name);
   CHECK_SILO_QUERY(
      maybe_target_column.has_value(),
      "The column '{}' is not contained in the database",
      column.name
   );
   auto target_column = maybe_target_column.value();

   return silo::storage::column::visit(target_column.type, [&]<storage::column::Column Column>() {
      const auto& value_column = table.columns.getColumns<Column>().at(column.name);
      return std::make_unique<filter::operators::IndexScan>(
         CopyOnWriteBitmap{&value_column.null_bitmap}, table.row_layout
      );
   });
}

}  // namespace silo::query_engine::scalar_expressions
