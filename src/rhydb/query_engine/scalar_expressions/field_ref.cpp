#include "rhydb/query_engine/scalar_expressions/field_ref.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <arrow/compute/api.h>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/filter/operators/operator.h"

namespace rhydb::query_engine::scalar_expressions {

FieldRef::FieldRef(schema::ColumnIdentifier column)
    : column(std::move(column)) {}

std::string FieldRef::toString() const {
   return column.name;
}

std::vector<schema::ColumnIdentifier> FieldRef::freeIUs() const {
   return {column};
}

arrow::Result<arrow::compute::Expression> FieldRef::toArrowExpression() const {
   return arrow::compute::field_ref(column.name);
}

std::unique_ptr<ScalarExpression> FieldRef::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<FieldRef>(column);
}

std::unique_ptr<filter::operators::Operator> FieldRef::compile(const storage::Table& /*table*/
) const {
   // A column reference is a scalar value, not a filter predicate.
   SILO_UNIMPLEMENTED();
}

}  // namespace rhydb::query_engine::scalar_expressions
