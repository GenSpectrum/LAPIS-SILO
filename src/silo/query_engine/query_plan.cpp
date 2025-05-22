#include "silo/query_engine/query_plan.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "arrow/acero/query_context.h"
#include "arrow/array.h"
#include "arrow/record_batch.h"

namespace silo::query_engine {

void silo::query_engine::QueryPlan::execute() {
   SPDLOG_TRACE("{}", arrow_plan->ToString());
   arrow_plan->StartProducing();
   auto future = arrow_plan->finished();
   future.Wait();
   if (future.status().ok()) {
      SPDLOG_DEBUG("All results successfully produced. Clearing ExecPlan.");
   } else {
      throw std::runtime_error(fmt::format(
         "Internal server error. Please notify developers. SILO constructed an invalid arrow plan. "
         "It is likely that more user-input validation needs to be added: {}",
         future.status().ToString()
      ));
   }
   arrow_plan->StopProducing();
}

}  // namespace silo::query_engine