#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

#include "silo/common/panic.h"

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;

   QueryPlan() {
      auto status = arrow::acero::ExecPlan::Make().Value(&arrow_plan);
      if (!status.ok()) {
         SILO_PANIC("Cannot create ExecPlan, Arrow error: {}", status.ToString());
      }
   }

   void execute();
};

}  // namespace silo::query_engine
