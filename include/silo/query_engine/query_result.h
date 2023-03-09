#ifndef SILO_QUERY_ENGINE_RESULT_H
#define SILO_QUERY_ENGINE_RESULT_H

#include <string>

namespace silo {
namespace response {
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
}  // namespace response
}  // namespace silo

#endif  // SILO_QUERY_ENGINE_RESULT_H
