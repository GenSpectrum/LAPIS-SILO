#include "silo/query_engine/query_engine.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>

#include "silo/common/block_timer.h"
#include "silo/common/log.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/database_partition.h"

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace silo::query_engine {

QueryEngine::QueryEngine(const silo::Database& database)
    : database(database) {}

QueryResult QueryEngine::executeQuery(const std::string& query_string) const {
   Query query(query_string);

   SPDLOG_DEBUG("Parsed query: {}", query.filter->toString(database));

   std::vector<std::string> compiled_queries(database.partitions.size());
   std::vector<silo::query_engine::OperatorResult> partition_filters(database.partitions.size());
   int64_t filter_time;
   {
      const BlockTimer timer(filter_time);
      const tbb::blocked_range<size_t> range(0, database.partitions.size(), 1);
      tbb::parallel_for(range.begin(), range.end(), [&](const size_t& partition_index) {
         std::unique_ptr<operators::Operator> part_filter = query.filter->compile(
            database,
            database.partitions[partition_index],
            silo::query_engine::filter_expressions::Expression::AmbiguityMode::NONE
         );
         compiled_queries[partition_index] = part_filter->toString();
         partition_filters[partition_index] = part_filter->evaluate();
      });
   }

   for (uint32_t i = 0; i < database.partitions.size(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }

   QueryResult query_result;
   int64_t action_time;
   {
      const BlockTimer timer(action_time);
      query_result = query.action->executeAndOrder(database, std::move(partition_filters));
   }

   LOG_PERFORMANCE("Query: {}", query_string);
   LOG_PERFORMANCE("Execution (filter): {} microseconds", std::to_string(filter_time));
   LOG_PERFORMANCE("Execution (action): {} microseconds", std::to_string(action_time));

   return query_result;
}

}  // namespace silo::query_engine
