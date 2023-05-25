#include "silo/query_engine/query_engine.h"

#include <tbb/parallel_for.h>
#include <memory>
#include <string>
#include <vector>

// query_parse_exception.h must be before the RAPIDJSON_ASSERT because it is used there
#include "silo/query_engine/query_parse_exception.h"
// Do not remove the next line. It overwrites the rapidjson abort, so it can throw an exception and
// does not abort.
#define RAPIDJSON_ASSERT(x)                                                    \
   if (!(x))                                                                   \
   throw silo::QueryParseException(                                            \
      "The query was not a valid JSON: " + std::string(RAPIDJSON_STRINGIFY(x)) \
   )
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>

#include "silo/common/block_timer.h"
#include "silo/common/log.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/parser.h"
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
   rapidjson::Document json_document;
   json_document.Parse(query.c_str());
   if (!json_document.HasMember("filterExpression") || !json_document["filterExpression"].IsObject() ||
       !json_document.HasMember("action") || !json_document["action"].IsObject()) {
      throw QueryParseException("Query json must contain filterExpression and action.");
   }

   std::vector<std::string> compiled_queries(database.partitions.size());

   response::QueryResult query_result;
   std::unique_ptr<filters::Expression> filter;
   {
      BlockTimer const timer(query_result.parse_time);
      filter = query_engine::parseExpression(database, json_document["filterExpression"], 0);
      SPDLOG_DEBUG("Parsed query: {}", filter->toString(database));
   }

   LOG_PERFORMANCE("Parse: {} microseconds", std::to_string(query_result.parse_time));

   std::vector<silo::query_engine::OperatorResult> partition_filters(database.partitions.size());
   {
      BlockTimer const timer(query_result.filter_time);
      tbb::blocked_range<size_t> const range(0, database.partitions.size(), 1);
      tbb::parallel_for(range.begin(), range.end(), [&](const size_t& partition_index) {
         std::unique_ptr<operators::Operator> part_filter =
            filter->compile(database, database.partitions[partition_index]);
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
      const auto& action = json_document["action"];
      CHECK_SILO_QUERY(
         action.HasMember("type"), "The field 'type' is required on a SILO query action"
      )
      CHECK_SILO_QUERY(
         action["type"].IsString(), "The field 'type' in a SILO query action needs to be a string"
      )
      const auto& action_type = action["type"].GetString();

      if (action.HasMember("groupByFields")) {
         CHECK_SILO_QUERY(
            action["groupByFields"].IsArray(),
            "The field 'type' in a SILO query action needs to be a string"
         )
         std::vector<std::string> group_by_fields;
         for (const auto& field : action["groupByFields"].GetArray()) {
            group_by_fields.emplace_back(field.GetString());
         }

         if (strcmp(action_type, "Aggregated") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Aggregated is not properly implemented yet"};
         } else if (strcmp(action_type, "List") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::List is not properly implemented yet"};
         } else if (strcmp(action_type, "Mutations") == 0) {
            query_result.query_result =
               response::ErrorResult{"groupByFields::Mutations is not properly implemented yet"};
         } else {
            query_result.query_result =
               response::ErrorResult{"groupByFields is not properly implemented yet"};
         }

      } else {
         if (strcmp(action_type, "Aggregated") == 0) {
            const unsigned count = executeCount(database, partition_filters);
            query_result.query_result = response::AggregationResult{count};
         } else if (strcmp(action_type, "List") == 0) {
         } else if (strcmp(action_type, "Mutations") == 0) {
            double min_proportion = DEFAULT_MINIMAL_PROPORTION;
            if (action.HasMember("minProportion") && action["minProportion"].IsDouble()) {
               if (action["minProportion"].GetDouble() <= 0.0) {
                  query_result.query_result = response::ErrorResult{
                     "Invalid proportion", "minProportion must be in interval (0.0,1.0]"};
                  return query_result;
               }
               min_proportion = action["minProportion"].GetDouble();
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
