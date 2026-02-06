#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>
#include <arrow/io/interfaces.h>

namespace silo::query_engine::exec_node {

class ArrowBatchSink {
  public:
   virtual ~ArrowBatchSink() = default;

   virtual arrow::Status writeBatch(const arrow::compute::ExecBatch& batch) = 0;
   virtual arrow::Status finish() = 0;
};

}  // namespace silo::query_engine::exec_node
