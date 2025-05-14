#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;

   QueryPlan() {
      arrow_plan = arrow::acero::ExecPlan::Make().ValueOrDie();  // TODO do not die
   }

   void execute();
};

}  // namespace silo::query_engine
