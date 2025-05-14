#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine {

class QueryPlan {
  public:
   std::shared_ptr<arrow::acero::ExecPlan> arrow_plan;

   QueryPlan() {
      arrow_plan = arrow::acero::ExecPlan::Make().ValueOrElse([]() -> std::shared_ptr<arrow::acero::ExecPlan> {throw std::runtime_error("Could not create ExecPlan"); SILO_UNREACHABLE(); }); // TODO error type
   }

   void execute();
};

}  // namespace silo::query_engine
