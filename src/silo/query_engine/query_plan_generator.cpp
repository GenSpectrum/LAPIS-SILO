#include "silo/query_engine/optimizer/query_plan_generator.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/ipc/writer.h>

#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/exec_node/legacy_result_producer.h"
#include "silo/query_engine/exec_node/select.h"

namespace silo::query_engine::optimizer {

QueryPlanGenerator::QueryPlanGenerator(std::shared_ptr<silo::Database> database)
    : database(database) {}

QueryPlan QueryPlanGenerator::createQueryPlan(
   std::shared_ptr<Query> query,
   std::ostream& output_stream
) {
   return query->toQueryPlan(database, output_stream);
}

}  // namespace silo::query_engine::optimizer