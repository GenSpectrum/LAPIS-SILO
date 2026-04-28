#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {

const double VALUE_IN_FILTER = 1.23;
const double VALUE_BELOW_FILTER = 0.345;
const double VALUE_ABOVE_FILTER = 2.345;

nlohmann::json createDataWithFloatValue(const std::string& primaryKey, double value) {
   return {
      {"primaryKey", primaryKey},
      {"float_value", value},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

nlohmann::json createDataWithFloatNullValue(const std::string& primaryKey) {
   return {
      {"primaryKey", primaryKey},
      {"float_value", nullptr},
      {"segment1", nullptr},
      {"unaligned_segment1", nullptr},
      {"gene1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithFloatValue("id_0", VALUE_IN_FILTER),
   createDataWithFloatValue("id_1", VALUE_IN_FILTER),
   createDataWithFloatValue("id_2", VALUE_BELOW_FILTER),
   createDataWithFloatValue("id_3", VALUE_ABOVE_FILTER),
   createDataWithFloatNullValue("id_4")
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "float_value"
      type: "float"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario FLOAT_EQUALS_VALUE_SCENARIO = {
   .name = "FLOAT_EQUALS_VALUE_SCENARIO",
   .query = "default.filter(float_value = 1.23).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}}}
   )
};

const QueryTestScenario NEGATED_FLOAT_EQUALS_VALUE_SCENARIO = {
   .name = "NEGATED_FLOAT_EQUALS_VALUE_SCENARIO",
   .query = "default.filter(!(float_value = 1.23)).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"float_value", nullptr}}}
   )
};

const QueryTestScenario FLOAT_EQUALS_NULL_SCENARIO = {
   .name = "FLOAT_EQUALS_NULL_SCENARIO",
   .query = "default.filter(float_value = null).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"float_value", nullptr}}})
};

const QueryTestScenario NEGATED_FLOAT_EQUALS_NULL_SCENARIO = {
   .name = "NEGATED_FLOAT_EQUALS_NULL_SCENARIO",
   .query = "default.filter(!(float_value = null)).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO",
   .query = "default.filter(float_value.between(0.5, 1.5)).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json({
      {{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
      {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
   })
};

const QueryTestScenario NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO",
   .query = "default.filter(!(float_value.between(0.5, 1.5))).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"float_value", nullptr}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "FLOAT_BETWEEN_WITH_FROM_SCENARIO",
   .query = "default.filter(float_value >= 0.5).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario NEGATED_FLOAT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "NEGATED_FLOAT_BETWEEN_WITH_FROM_SCENARIO",
   .query = "default.filter(!(float_value >= 0.5)).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_4"}, {"float_value", nullptr}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "FLOAT_BETWEEN_WITH_TO_SCENARIO",
   .query = "default.filter(float_value <= 1.5).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}}}
   )
};

const QueryTestScenario NEGATED_FLOAT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "NEGATED_FLOAT_BETWEEN_WITH_TO_SCENARIO",
   .query = "default.filter(!(float_value <= 1.5)).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"float_value", nullptr}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO",
   .query = "default.filter(float_value.isNotNull()).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO",
   .query = "default.filter(!(float_value.isNotNull())).project({primaryKey, float_value})",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"float_value", nullptr}}})
};

}  // namespace

QUERY_TEST(
   FloatEqualsTest,
   TEST_DATA,
   ::testing::Values(
      FLOAT_EQUALS_VALUE_SCENARIO,
      NEGATED_FLOAT_EQUALS_VALUE_SCENARIO,
      FLOAT_EQUALS_NULL_SCENARIO,
      NEGATED_FLOAT_EQUALS_NULL_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_SCENARIO,
      NEGATED_FLOAT_BETWEEN_WITH_FROM_SCENARIO,
      FLOAT_BETWEEN_WITH_TO_SCENARIO,
      NEGATED_FLOAT_BETWEEN_WITH_TO_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO,
      NEGATED_FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO
   )
);
