#pragma once

#include <ostream>

#include <arrow/acero/exec_plan.h>

namespace silo::query_engine {

class QueryPlan {
  public:
   arrow::acero::Declaration declaration;

   void writeToSink(std::ostream& output);
};

}  // namespace silo::query_engine
