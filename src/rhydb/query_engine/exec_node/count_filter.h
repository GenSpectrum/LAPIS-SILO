#pragma once

#include <memory>

#include <arrow/acero/exec_plan.h>
#include <arrow/type_fwd.h>

#include "rhydb/query_engine/operators/compute_filter.h"

namespace rhydb::query_engine::exec_node {

/// Adds a source node to `plan` that produces a single row with the number of rows matched by
/// `filter`, in the only column of `output_schema`.
arrow::acero::ExecNode* addCountFilterNode(
   arrow::acero::ExecPlan& plan,
   operators::CompiledFilter filter,
   std::shared_ptr<arrow::Schema> output_schema
);

}  // namespace rhydb::query_engine::exec_node
