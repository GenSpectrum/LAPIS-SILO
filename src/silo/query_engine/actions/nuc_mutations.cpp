#include "silo/query_engine/actions/nuc_mutations.h"

#include <nlohmann/json.hpp>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

NucMutations::NucMutations(double min_proportion)
    : min_proportion(min_proportion) {}

QueryResult NucMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   if (min_proportion <= 0 || min_proportion > 1) {
      return {ErrorResult{"Invalid proportion", "minProportion must be in interval (0.0, 1.0]"}};
   }
   return {ErrorResult{"Not implemented", "The NucMutations action has not been implemented"}};
}

const double DEFAULT_MIN_PROPORTION = 0.02;

void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action) {
   double min_proportion = DEFAULT_MIN_PROPORTION;
   if (json.contains("minProportion")) {
      min_proportion = json["minProportion"].get<double>();
   }
   action = std::make_unique<NucMutations>(min_proportion);
}

}  // namespace silo::query_engine::actions
