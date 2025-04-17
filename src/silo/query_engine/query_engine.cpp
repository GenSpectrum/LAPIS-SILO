#include "silo/query_engine/query_engine.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>

#include "silo/common/block_timer.h"
#include "silo/common/log.h"
#include "silo/database.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine {

using filter::expressions::Expression;
using filter::operators::Operator;

QueryResult executeQuery(const Database& database, const std::string& query_string) {
   Query query(query_string);

   SPDLOG_DEBUG("Parsed query: {}", query.filter->toString());

   std::vector<std::string> compiled_queries(database.table->getNumberOfPartitions());
   std::vector<CopyOnWriteBitmap> partition_filters(database.table->getNumberOfPartitions());
   int64_t filter_time;
   {
      const silo::common::BlockTimer timer(filter_time);
      for (size_t partition_index = 0; partition_index != database.table->getNumberOfPartitions();
           partition_index++) {
         std::unique_ptr<Operator> part_filter = query.filter->compile(
            database, database.table->getPartition(partition_index), Expression::AmbiguityMode::NONE
         );
         compiled_queries[partition_index] = part_filter->toString();
         partition_filters[partition_index] = part_filter->evaluate();
      }
   }

   for (uint32_t i = 0; i < database.table->getNumberOfPartitions(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }

   QueryResult query_result;
   int64_t action_time;
   {
      const silo::common::BlockTimer timer(action_time);
      query_result = query.action->executeAndOrder(database, std::move(partition_filters));
   }

   LOG_PERFORMANCE("Query: {}", query_string);
   LOG_PERFORMANCE("Execution (filter): {} microseconds", std::to_string(filter_time));
   LOG_PERFORMANCE("Execution (action): {} microseconds", std::to_string(action_time));

   return query_result;
}

}  // namespace silo::query_engine
