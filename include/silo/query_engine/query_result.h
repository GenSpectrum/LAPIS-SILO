#ifndef SILO_QUERY_ENGINE_RESULT_H
#define SILO_QUERY_ENGINE_RESULT_H

#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

namespace silo::response {

struct AggregationResult {
   int64_t count;
};

struct MutationProportion {
   std::string mutation;
   double proportion;
   int64_t count;
};

struct ErrorResult {
   std::string error;
   std::string message;
};

struct QueryResult {
   std::variant<AggregationResult, std::vector<MutationProportion>, ErrorResult> query_result;
   int64_t parse_time;
   int64_t filter_time;
   int64_t action_time;
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& json, const AggregationResult& aggregation_result);
void to_json(nlohmann::json& json, const MutationProportion& mutation_proportion);
void to_json(nlohmann::json& json, const ErrorResult& error_result);
void to_json(nlohmann::json& json, const QueryResult& query_result);
// NOLINTEND(readability-identifier-naming)

}  // namespace silo::response

#endif  // SILO_QUERY_ENGINE_RESULT_H
