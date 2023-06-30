#ifndef SILO_QUERY_ENGINE_RESULT_H
#define SILO_QUERY_ENGINE_RESULT_H

#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {

struct AggregationResult {
   std::map<std::string, std::variant<std::string, int32_t, double>> fields;
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
   std::variant<std::vector<AggregationResult>, std::vector<MutationProportion>, ErrorResult>
      query_result;
};

// NOLINTBEGIN(readability-identifier-naming)
void to_json(nlohmann::json& json, const AggregationResult& aggregation_result);
void to_json(nlohmann::json& json, const MutationProportion& mutation_proportion);
void to_json(nlohmann::json& json, const ErrorResult& error_result);
void to_json(nlohmann::json& json, const QueryResult& query_result);
// NOLINTEND(readability-identifier-naming)

}  // namespace silo::query_engine

#endif  // SILO_QUERY_ENGINE_RESULT_H
