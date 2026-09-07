#pragma once

#include <cstdint>

#include <arrow/acero/exec_plan.h>
#include <arrow/compute/ordering.h>
#include <arrow/result.h>

namespace rhydb::query_engine::exec_node {

/// Appends a select-k Acero node on top of `input_node` and returns it.
///
/// The node (a custom `arrow::acero::ExecNode` registered as `rhydb_select_k`) buffers the whole
/// input stream, selects the `offset + limit` smallest rows (by `ordering`) with a heap-based
/// `select_k`, and emits the `[offset, offset + limit)` window in sorted order.
arrow::Result<arrow::acero::ExecNode*> addSelectKNode(
   arrow::acero::ExecPlan& plan,
   arrow::acero::ExecNode* input_node,
   const arrow::Ordering& ordering,
   uint32_t offset,
   uint32_t limit
);

}  // namespace rhydb::query_engine::exec_node
