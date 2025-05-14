#pragma once

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/compute/api_vector.h>
#include <arrow/record_batch.h>
#include <spdlog/spdlog.h>

#include "silo/query_engine/query.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::exec_node {

class Sort : public arrow::acero::ExecNode {
   std::vector<std::shared_ptr<arrow::RecordBatch>> lazy_queue;
};

}  // namespace silo::query_engine::exec_node
