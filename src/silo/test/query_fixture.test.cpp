#include "silo/test/query_fixture.test.h"

std::string silo::test::printScenarioName(
   const ::testing::TestParamInfo<QueryTestScenario>& scenario
) {
   return scenario.param.name;
}
