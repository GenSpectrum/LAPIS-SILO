#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;

   QueryPlan() {
      auto tmp = arrow::acero::ExecPlan::Make();
      if (tmp.ok()) {
         arrow_plan = tmp.ValueUnsafe();
      } else {
         throw std::runtime_error("Could not create ExecPlan");
      }
   }

   void execute();
};

}  // namespace silo::query_engine
