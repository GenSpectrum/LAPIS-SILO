#ifndef SILO_QUERY_ENGINE_RESULT_H
#define SILO_QUERY_ENGINE_RESULT_H

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
   std::variant<AggregationResult, std::vector<MutationProportion>, ErrorResult> queryResult;
   int64_t parseTime;   // NOLINT
   int64_t filterTime;  // NOLINT
   int64_t actionTime;  // NOLINT
};
}  // namespace silo::response

#endif  // SILO_QUERY_ENGINE_RESULT_H
