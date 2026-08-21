#pragma once

#include <arrow/compute/expression.h>
#include <arrow/result.h>

#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"

namespace rhydb::query_engine::exec_node {

/// Translates a scalar expression into an Arrow compute expression, e.g. for use in a projection or
/// for direct evaluation over a materialized batch. Field references become named `field_ref`s
/// (resolved when the expression is bound to a schema); function calls like `at`, `isoWeek` and
/// zstd-decompress map to the corresponding Arrow compute calls. Returns `NotImplemented` for
/// expressions with no Arrow translation.
arrow::Result<arrow::compute::Expression> scalarToArrowExpression(
   const scalar_expressions::ScalarExpression& expression
);

}  // namespace rhydb::query_engine::exec_node
