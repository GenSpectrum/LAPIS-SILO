#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const int VALUE_IN_FILTER = 3;
const int VALUE_BELOW_FILTER = 1;
const int VALUE_ABOVE_FILTER = 5;
const int BELOW_FILTER = 2;
const int ABOVE_FILTER = 4;

nlohmann::json createDataWithIntValue(const std::string& primaryKey, int value) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"int_value", value}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

nlohmann::json createDataWithIntNullValue(const std::string& primaryKey) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"int_value", nullptr}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithIntValue("id_0", VALUE_IN_FILTER),
   createDataWithIntValue("id_1", VALUE_IN_FILTER),
   createDataWithIntValue("id_2", VALUE_BELOW_FILTER),
   createDataWithIntValue("id_3", VALUE_ABOVE_FILTER),
   createDataWithIntNullValue("id_4")
};

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata =
          {{.name = "primaryKey", .type = ValueType::STRING},
           {.name = "int_value", .type = ValueType::INT}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA},
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
   .name = "intEqualsValue",
   .query = createIntEqualsQuery("int_value", VALUE_IN_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}}}
   )
};

const QueryTestScenario INT_EQUALS_NULL_SCENARIO = {
   .name = "intEqualsNull",
   .query = createIntEqualsQuery("int_value", nullptr),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"int_value", nullptr}}})
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "intBetweenWithFromAndTo",
   .query = createIntBetweenQuery("int_value", BELOW_FILTER, ABOVE_FILTER),
   .expected_query_result = nlohmann::json({
      {{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
      {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
   })
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "intBetweenWithFrom",
   .query = createIntBetweenQuery("int_value", BELOW_FILTER, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "intBetweenWithTo",
   .query = createIntBetweenQuery("int_value", nullptr, ABOVE_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}}}
   )
};

const QueryTestScenario INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "intBetweenWithFromAndToNull",
   .query = createIntBetweenQuery("int_value", nullptr, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"int_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"int_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"int_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario INT_EQUALS_WITH_INVALID_VALUE = {
   .name = "intEqualsWithInvalidValue",
   .query = createIntEqualsQuery("int_value", INT32_MIN),
   .expected_error_message =
      "The field 'value' in an IntEquals expression must be an integer in [-2147483647; "
      "2147483647] or null"
};

const QueryTestScenario INT_BETWEEN_WITH_INVALID_FROM_VALUE = {
   .name = "intBetweenWithInvalidFromValue",
   .query = createIntBetweenQuery("int_value", INT32_MIN, 1),
   .expected_error_message =
      "The field 'from' in an IntBetween expression must be an integer in [-2147483647; "
      "2147483647] or null"
};

const QueryTestScenario INT_BETWEEN_WITH_INVALID_TO_VALUE = {
   .name = "intBetweenWithInvalidToValue",
   .query = createIntBetweenQuery("int_value", 0, INT32_MIN),
   .expected_error_message =
      "The field 'to' in an IntBetween expression must be an integer in [-2147483647; 2147483647] "
      "or null"
};
}  // namespace

QUERY_TEST(
   IntEqualsTest,
   TEST_DATA,
   ::testing::Values(
      INT_EQUALS_VALUE_SCENARIO,
      INT_EQUALS_NULL_SCENARIO,
      INT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      INT_BETWEEN_WITH_FROM_SCENARIO,
      INT_BETWEEN_WITH_TO_SCENARIO,
      INT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO,
      INT_EQUALS_WITH_INVALID_VALUE,
      INT_BETWEEN_WITH_INVALID_FROM_VALUE,
      INT_BETWEEN_WITH_INVALID_TO_VALUE
   )
);
