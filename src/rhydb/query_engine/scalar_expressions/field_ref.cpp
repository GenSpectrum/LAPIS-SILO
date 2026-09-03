#include "rhydb/query_engine/scalar_expressions/field_ref.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/filter/operators/index_scan.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/illegal_query_exception.h"
#include "rhydb/storage/column/bool_column.h"

namespace rhydb::query_engine::scalar_expressions {

FieldRef::FieldRef(schema::ColumnIdentifier column)
    : column(std::move(column)) {}

std::string FieldRef::toString() const {
   return column.name;
}

std::vector<schema::ColumnIdentifier> FieldRef::freeIUs() const {
   return {column};
}

std::unique_ptr<ScalarExpression> FieldRef::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FieldRef>(column);
}

std::unique_ptr<filter::operators::Operator> FieldRef::compile(const storage::Table& table) const {
   CHECK_RHYDB_QUERY(
      table.columns.bool_columns.contains(column.name),
      "The column '{}' is not of type bool and cannot be used directly as a filter predicate",
      column.name
   );
   const auto& bool_column = table.columns.bool_columns.at(column.name);
   return std::make_unique<filter::operators::IndexScan>(
      CopyOnWriteBitmap{&bool_column.true_bitmap}, table.row_layout
   );
}

}  // namespace rhydb::query_engine::scalar_expressions
