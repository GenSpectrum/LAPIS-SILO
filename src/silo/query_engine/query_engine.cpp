#include "silo/query_engine/query_engine.h"

#include <tbb/parallel_for.h>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/log.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter_expressions/expression.h"
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
      BlockTimer const timer(filter_time);
      tbb::blocked_range<size_t> const range(0, database.partitions.size(), 1);
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

   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }
   LOG_PERFORMANCE("Execution (filter): {} microseconds", std::to_string(filter_time));

   QueryResult query_result;
   int64_t action_time;
   {
      BlockTimer const timer(action_time);
      query_result = query.action->execute(database, std::move(partition_filters));
   }

   LOG_PERFORMANCE("Execution (action): {} microseconds", std::to_string(action_time));

   return query_result;
}

}  // namespace silo::query_engine
