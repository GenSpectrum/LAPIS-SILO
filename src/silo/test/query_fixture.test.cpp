#include "silo/test/query_fixture.test.h"

namespace silo::test {

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario) {
   return scenario.param.name;
}

nlohmann::json negateFilter(const nlohmann::json& query) {
   return nlohmann::json{
      {"action", query["action"]},
      {"filterExpression", {{"type", "Not"}, {"child", query["filterExpression"]}}}
   };
}

}  // namespace silo::test
