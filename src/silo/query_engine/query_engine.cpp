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
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"

#define CHECK_SILO_QUERY(condition, message)    \
   if (!(condition)) {                          \
      throw silo::QueryParseException(message); \
   }

namespace filters = silo::query_engine::filter_expressions;
namespace operators = silo::query_engine::operators;

namespace silo {

QueryEngine::QueryEngine(const silo::Database& database)
    : database(database) {}

// TODO(someone): reduce cognitive complexity
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
response::QueryResult QueryEngine::executeQuery(const std::string& query) const {
   nlohmann::json json;
   try {
      json = nlohmann::json::parse(query);
   } catch (const QueryParseException& qe) {
      throw qe;
   } catch (const nlohmann::json::parse_error& ex) {
      throw QueryParseException("The query was not a valid JSON: " + std::string(ex.what()));
   } catch (const nlohmann::json::exception& ex) {
      throw QueryParseException("The query was not a valid JSON: " + std::string(ex.what()));
   }
   if (!json.contains("filterExpression") || !json["filterExpression"].is_object() ||
       !json.contains("action") || !json["action"].is_object()) {
      throw QueryParseException("Query json must contain filterExpression and action.");
   }

   std::vector<std::string> compiled_queries(database.partitions.size());

   response::QueryResult query_result;
   std::unique_ptr<silo::query_engine::filter_expressions::Expression> filter;
   {
      BlockTimer const timer(query_result.parse_time);
      filter = json["filterExpression"]
                  .get<std::unique_ptr<silo::query_engine::filter_expressions::Expression>>();
      SPDLOG_DEBUG("Parsed query: {}", filter->toString(database));
   }

   LOG_PERFORMANCE("Parse: {} microseconds", std::to_string(query_result.parse_time));

   std::vector<silo::query_engine::OperatorResult> partition_filters(database.partitions.size());
   {
      BlockTimer const timer(query_result.filter_time);
      tbb::blocked_range<size_t> const range(0, database.partitions.size(), 1);
      tbb::parallel_for(range.begin(), range.end(), [&](const size_t& partition_index) {
         std::unique_ptr<operators::Operator> part_filter = filter->compile(
            database, database.partitions[partition_index], filters::Expression::AmbiguityMode::NONE
         );
         compiled_queries[partition_index] = part_filter->toString();
         partition_filters[partition_index] = part_filter->evaluate();
      });
   }
   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      SPDLOG_DEBUG("Simplified query for partition {}: {}", i, compiled_queries[i]);
   }
   LOG_PERFORMANCE("Execution (filter): {} microseconds", std::to_string(query_result.filter_time));

   {
      BlockTimer const timer(query_result.action_time);
      const auto& action = json["action"];
      CHECK_SILO_QUERY(
         action.contains("type"), "The field 'type' is required on a SILO query action"
      )
      CHECK_SILO_QUERY(
         action["type"].is_string(), "The field 'type' in a SILO query action needs to be a string"
      )
      const std::string& action_type = action["type"];

      if (action.contains("groupByFields")) {
         CHECK_SILO_QUERY(
            action["groupByFields"].is_array(),
            "The field 'type' in a SILO query action needs to be a string"
         )
         std::vector<std::string> group_by_fields;
         for (const auto& field : action["groupByFields"]) {
            group_by_fields.emplace_back(field);
         }

         if (action_type == "Aggregated") {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Aggregated is not properly implemented yet"};
         } else if (action_type == "List") {
            query_result.query_result =
               response::ErrorResult{"groupByFields::List is not properly implemented yet"};
         } else if (action_type == "Mutations") {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Mutations is not properly implemented yet"};
         } else {
            query_result.query_result =
               response::ErrorResult{"groupByFields is not properly implemented yet"};
         }

      } else {
         if (action_type == "Aggregated") {
            const unsigned count = executeCount(database, partition_filters);
            query_result.query_result = response::AggregationResult{count};
         } else if (action_type == "List") {
         } else if (action_type == "Mutations") {
            double min_proportion = DEFAULT_MINIMAL_PROPORTION;
            if (action.contains("minProportion") && action["minProportion"].is_number_float()) {
               if (action["minProportion"].get<double>() <= 0.0) {
                  query_result.query_result = response::ErrorResult{
                     "Invalid proportion", "minProportion must be in interval (0.0,1.0]"};
                  return query_result;
               }
               min_proportion = action["minProportion"];
            }
            std::vector<MutationProportion> mutations =
               executeMutations(database, partition_filters, min_proportion);

            std::vector<response::MutationProportion> output_mutation_proportions(mutations.size());
            std::transform(
               mutations.begin(),
               mutations.end(),
               output_mutation_proportions.begin(),
               [](MutationProportion mutation_proportion) {
                  return response::MutationProportion{
                     mutation_proportion.mutation_from +
                        std::to_string(mutation_proportion.position) +
                        mutation_proportion.mutation_to,
                     mutation_proportion.proportion,
                     mutation_proportion.count};
               }
            );
            query_result.query_result = output_mutation_proportions;
         } else {
            query_result.query_result = response::ErrorResult{
               "Unknown action", std::string(action_type) + " is not a valid action"};
         }
      }
   }

   LOG_PERFORMANCE("Execution (action): {} microseconds", std::to_string(query_result.action_time));

   return query_result;
}

}  // namespace silo
