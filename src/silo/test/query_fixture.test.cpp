#include "silo/test/query_fixture.test.h"

namespace silo::test {

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario) {
   return scenario.param.name;
}

}  // namespace silo::test
