#pragma once

#include <functional>
#include <optional>
#include <ostream>

#include <arrow/acero/exec_plan.h>
#include <arrow/util/future.h>

#include "silo/common/panic.h"

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;
   // The function which returns the exec batches in the correct order of the output
   std::shared_ptr<arrow::Schema> results_schema;
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> results_generator;

   static arrow::Result<QueryPlan> makeQueryPlan(
      std::shared_ptr<arrow::acero::ExecPlan> arrow_plan,
      arrow::acero::ExecNode* root
   );

   void executeAndWrite(std::ostream* output_stream);

  private:
   QueryPlan(std::shared_ptr<arrow::acero::ExecPlan> arrow_plan)
       : arrow_plan(arrow_plan) {}

   arrow::Status executeAndWriteImpl(std::ostream* output_stream);
};

}  // namespace silo::query_engine
