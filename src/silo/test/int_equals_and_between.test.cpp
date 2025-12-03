#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::negateFilter;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const int VALUE_IN_FILTER = 3;
const int VALUE_BELOW_FILTER = 1;
const int VALUE_ABOVE_FILTER = 5;
const int BELOW_FILTER = 2;
const int ABOVE_FILTER = 4;

nlohmann::json createDataWithIntValue(const std::string& primaryKey, int value) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", value},
      {"segment1", nullptr},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr}
   };
}

nlohmann::json createDataWithIntNullValue(const std::string& primaryKey) {
   return {
      {"primaryKey", primaryKey},
      {"int_value", nullptr},
      {"segment1", nullptr},
      {"gene1", nullptr},
      {"unaligned_segment1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithIntValue("id_0", VALUE_IN_FILTER),
   createDataWithIntValue("id_1", VALUE_IN_FILTER),
   createDataWithIntValue("id_2", VALUE_BELOW_FILTER),
   createDataWithIntValue("id_3", VALUE_ABOVE_FILTER),
   createDataWithIntNullValue("id_4")
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "int_value"
      type: "int"
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

nlohmann::json createIntEqualsQuery(const std::string& column, const nlohmann::json value) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression", {{"type", "IntEquals"}, {"column", column}, {"value", value}}}
   };
}

nlohmann::json createIntBetweenQuery(
   const std::string& column,
   const nlohmann::json from_value,
   const nlohmann::json to_value
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "IntBetween"}, {"column", column}, {"from", from_value}, {"to", to_value}}}
   };
}

const QueryTestScenario INT_EQUALS_VALUE_SCENARIO = {
   .name = "INT_EQUALS_VALUE_SCENARIO",
   .query = createIntEqualsQuery("int_value", VALUE_IN_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}}}
   )
};

const QueryTestScenario NEGATED_INT_EQUALS_VALUE_SCENARIO = {
   .name = "NEGATED_INT_EQUALS_VALUE_SCENARIO",
   .query = negateFilter(createIntEqualsQuery("int_value", VALUE_IN_FILTER)),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"int_value", nullptr}}}
   )
};

const QueryTestScenario INT_EQUALS_NULL_SCENARIO = {
   .name = "INT_EQUALS_NULL_SCENARIO",
   .query = createIntEqualsQuery("int_value", nullptr),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"int_value", nullptr}}})
};

const QueryTestScenario NEGATED_INT_EQUALS_NULL_SCENARIO = {
   .name = "NEGATED_INT_EQUALS_NULL_SCENARIO",
   .query = negateFilter(createIntEqualsQuery("int_value", nullptr)),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO",
   .query = createIntBetweenQuery("int_value", BELOW_FILTER, ABOVE_FILTER),
   .expected_query_result = nlohmann::json({
      {{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
      {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
   })
};

const QueryTestScenario NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO",
   .query = negateFilter(createIntBetweenQuery("int_value", BELOW_FILTER, ABOVE_FILTER)),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"int_value", nullptr}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "INT_BETWEEN_WITH_FROM_SCENARIO",
   .query = createIntBetweenQuery("int_value", BELOW_FILTER, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario NEGATED_INT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "NEGATED_INT_BETWEEN_WITH_FROM_SCENARIO",
   .query = negateFilter(createIntBetweenQuery("int_value", BELOW_FILTER, nullptr)),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_4"}, {"int_value", nullptr}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "INT_BETWEEN_WITH_TO_SCENARIO",
   .query = createIntBetweenQuery("int_value", nullptr, ABOVE_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}}}
   )
};

const QueryTestScenario NEGATED_INT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "NEGATED_INT_BETWEEN_WITH_TO_SCENARIO",
   .query = negateFilter(createIntBetweenQuery("int_value", nullptr, ABOVE_FILTER)),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}},
       {{"primaryKey", "id_4"}, {"int_value", nullptr}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO",
   .query = createIntBetweenQuery("int_value", nullptr, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO",
   .query = negateFilter(createIntBetweenQuery("int_value", nullptr, nullptr)),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"int_value", nullptr}}})
};

const QueryTestScenario INT_EQUALS_WITH_INVALID_VALUE = {
   .name = "INT_EQUALS_WITH_INVALID_VALUE",
   .query = createIntEqualsQuery("int_value", 0.3),
   .expected_error_message =
      "The field 'value' in an IntEquals expression must be an integer in [-2147483648; "
      "2147483647] or null"
};

const QueryTestScenario INT_BETWEEN_WITH_INVALID_FROM_VALUE = {
   .name = "INT_BETWEEN_WITH_INVALID_FROM_VALUE",
   .query = createIntBetweenQuery("int_value", false, 1),
   .expected_error_message =
      "The field 'from' in an IntBetween expression must be an integer in [-2147483648; "
      "2147483647] or null"
};

const QueryTestScenario INT_BETWEEN_WITH_INVALID_TO_VALUE = {
   .name = "INT_BETWEEN_WITH_INVALID_TO_VALUE",
   .query = createIntBetweenQuery("int_value", 0, "test"),
   .expected_error_message =
      "The field 'to' in an IntBetween expression must be an integer in [-2147483648; 2147483647] "
      "or null"
};
}  // namespace

QUERY_TEST(
   IntEqualsTest,
   TEST_DATA,
   ::testing::Values(
      INT_EQUALS_VALUE_SCENARIO,
      NEGATED_INT_EQUALS_VALUE_SCENARIO,
      INT_EQUALS_NULL_SCENARIO,
      NEGATED_INT_EQUALS_NULL_SCENARIO,
      INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      INT_BETWEEN_WITH_FROM_SCENARIO,
      NEGATED_INT_BETWEEN_WITH_FROM_SCENARIO,
      INT_BETWEEN_WITH_TO_SCENARIO,
      NEGATED_INT_BETWEEN_WITH_TO_SCENARIO,
      INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO,
      NEGATED_INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO,
      INT_EQUALS_WITH_INVALID_VALUE,
      INT_BETWEEN_WITH_INVALID_FROM_VALUE,
      INT_BETWEEN_WITH_INVALID_TO_VALUE
   )
);
