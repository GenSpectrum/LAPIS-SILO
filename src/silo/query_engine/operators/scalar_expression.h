#pragma once

#include <memory>

#include "silo/query_engine/filter/expressions/expression.h"

namespace silo::query_engine::operators {

/// A scalar expression evaluates to a single value per row (e.g. the value
/// assigned to a column in a `map()`, see MapNode). Every scalar expression is a
/// filter::expressions::Expression; its type() gives the produced ColumnType. A
/// boolean-typed scalar expression is a filter predicate, and literal values are
/// represented by the Int64Literal/FloatLiteral/StringLiteral/BoolLiteral
/// subclasses (see filter/expressions/literal.h).
using ScalarExpression = std::unique_ptr<filter::expressions::Expression>;

}  // namespace silo::query_engine::operators
