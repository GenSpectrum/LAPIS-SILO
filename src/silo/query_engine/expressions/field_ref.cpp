#include "silo/query_engine/expressions/field_ref.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "silo/common/panic.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

FieldRef::FieldRef(schema::ColumnIdentifier column)
    : column(std::move(column)) {}

std::string FieldRef::toString() const {
   return column.name;
}

std::vector<schema::ColumnIdentifier> FieldRef::freeIUs() const {
   return {column};
}

bool FieldRef::operator==(const Expression& other) const {
   const auto* other_casted = dynamic_cast<const FieldRef*>(&other);
   return other_casted != nullptr && column == other_casted->column;
}

std::unique_ptr<Expression> FieldRef::rewrite(
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

}  // namespace silo::query_engine::expressions
