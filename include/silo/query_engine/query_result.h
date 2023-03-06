#include <optional>

namespace silo {
namespace response {
struct aggregation_result {
   int64_t count;
};

struct mutation_proportion {
   std::string mutation;
   double proportion;
   int64_t count;
};
}
}
