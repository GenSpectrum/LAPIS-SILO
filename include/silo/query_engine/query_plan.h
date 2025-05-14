#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;

   QueryPlan() { auto status = arrow::acero::ExecPlan::Make().Value(&arrow_plan); }

   void execute();
};

}  // namespace silo::query_engine
