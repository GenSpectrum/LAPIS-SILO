#pragma once

#include <cstddef>
#include <string>

#include <arrow/acero/exec_plan.h>
#include <arrow/result.h>

namespace silo::query_engine::operators {

/// Name of the transient uint64 column that carries the per-row random hash used to implement
/// `randomize`. It is appended to the stream, sorted on, and projected back out again, so it is
/// never visible in a query result.
extern const std::string RANDOMIZE_HASH_FIELD_NAME;

/// Appends a `RANDOMIZE_HASH_FIELD_NAME` column holding `hash64(row_index, randomize_seed)` to the
/// stream, so a subsequent sort/select on that column yields a deterministic pseudo-random order.
arrow::Result<arrow::acero::ExecNode*> addRandomizeColumn(
   arrow::acero::ExecPlan& plan,
   arrow::acero::ExecNode* top_node,
   size_t randomize_seed
);

/// Projects the `RANDOMIZE_HASH_FIELD_NAME` column back out of the stream, undoing
/// `addRandomizeColumn` once the random order has been applied.
arrow::Result<arrow::acero::ExecNode*> removeRandomizeColumn(
   arrow::acero::ExecPlan& plan,
   arrow::acero::ExecNode* top_node
);

}  // namespace silo::query_engine::operators
