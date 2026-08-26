#pragma once

#include <memory>
#include <sstream>

#include <arrow/type_fwd.h>

#include "rhydb/append/table_inserter.h"
#include "rhydb/query_engine/exec_node/arrow_batch_sink.h"
#include "rhydb/query_engine/exec_node/ndjson_sink.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::command {

/// Query result sink that inserts the rows it receives into a table as they arrive: every batch is
/// serialized to NDJSON and handed to the append path right away, so only one batch is held in
/// memory instead of the whole result. Reusing the NDJSON representation keeps a single JSON
/// representation shared by the query and append sides, so every column type that a query can emit
/// and the append path can ingest round-trips without a bespoke Arrow-to-column bridge.
///
/// The rows become visible in the table when `finish()` commits them, which the query plan calls
/// once all batches have been written. A batch that cannot be inserted (e.g. a missing target
/// column) throws an AppendException out of `writeBatch`, leaving the insert uncommitted.
class TableInsertSink : public exec_node::ArrowBatchSink {
   std::stringstream batch_buffer;
   exec_node::NdjsonSink ndjson_sink;
   append::TableInserter table_inserter;
   append::NdjsonInsertStream insert_stream;

  public:
   TableInsertSink(
      std::shared_ptr<storage::Table> target_table,
      std::shared_ptr<arrow::Schema> results_schema
   );

   arrow::Status writeBatch(const arrow::compute::ExecBatch& batch) override;

   arrow::Status finish() override;
};

}  // namespace rhydb::query_engine::command
