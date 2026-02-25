#include "silo/test/query_fixture.test.h"

#include <iostream>
#include <sstream>
#include <string>

#include "silo/query_engine/exec_node/ndjson_sink.h"

namespace silo::test {

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario) {
   return scenario.param.name;
}

nlohmann::json executeQueryToJsonArray(
   query_engine::QueryPlan& query_plan,
   uint64_t timeout_in_seconds
) {
   std::stringstream buffer;
   query_engine::exec_node::NdjsonSink output_sink{&buffer, query_plan.results_schema};
   query_plan.executeAndWrite(output_sink, timeout_in_seconds);
   nlohmann::json result = nlohmann::json::array();
   std::string line;
   while (std::getline(buffer, line)) {
      auto line_object = nlohmann::json::parse(line);
      std::cout << line_object.dump() << '\n';
      result.push_back(line_object);
   }
   return result;
}

}  // namespace silo::test
