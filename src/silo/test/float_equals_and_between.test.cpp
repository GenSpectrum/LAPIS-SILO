#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {

const double VALUE_IN_FILTER = 1.23;
const double VALUE_BELOW_FILTER = 0.345;
const double VALUE_ABOVE_FILTER = 2.345;
const double BELOW_FILTER = 0.5;
const double ABOVE_FILTER = 1.5;

nlohmann::json createDataWithFloatValue(const std::string& primaryKey, double value) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"float_value", value}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

nlohmann::json createDataWithFloatNullValue(const std::string& primaryKey) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"float_value", nullptr}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}
const std::vector<nlohmann::json> DATA = {
   createDataWithFloatValue("id_0", VALUE_IN_FILTER),
   createDataWithFloatValue("id_1", VALUE_IN_FILTER),
   createDataWithFloatValue("id_2", VALUE_BELOW_FILTER),
   createDataWithFloatValue("id_3", VALUE_ABOVE_FILTER),
   createDataWithFloatNullValue("id_4")
};

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata =
          {{.name = "primaryKey", .type = ValueType::STRING},
           {.name = "float_value", .type = ValueType::FLOAT}},
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

nlohmann::json createFloatEqualsQuery(const std::string& column, const nlohmann::json value) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression", {{"type", "FloatEquals"}, {"column", column}, {"value", value}}}
   };
}

nlohmann::json createFloatBetweenQuery(
   const std::string& column,
   const nlohmann::json from_value,
   const nlohmann::json to_value
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "FloatBetween"}, {"column", column}, {"from", from_value}, {"to", to_value}}}
   };
}

const QueryTestScenario FLOAT_EQUALS_VALUE_SCENARIO = {
   .name = "floatEqualsValue",
   .query = createFloatEqualsQuery("float_value", VALUE_IN_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}}}
   )
};

const QueryTestScenario FLOAT_EQUALS_NULL_SCENARIO = {
   .name = "floatEqualsNull",
   .query = createFloatEqualsQuery("float_value", nullptr),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}, {"float_value", nullptr}}})
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO = {
   .name = "floatBetweenWithFromAndTo",
   .query = createFloatBetweenQuery("float_value", BELOW_FILTER, ABOVE_FILTER),
   .expected_query_result = nlohmann::json({
      {{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
      {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
   })
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_SCENARIO = {
   .name = "floatBetweenWithFrom",
   .query = createFloatBetweenQuery("float_value", BELOW_FILTER, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_TO_SCENARIO = {
   .name = "floatBetweenWithTo",
   .query = createFloatBetweenQuery("float_value", nullptr, ABOVE_FILTER),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}}}
   )
};

const QueryTestScenario FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO = {
   .name = "floatBetweenWithFromAndToNull",
   .query = createFloatBetweenQuery("float_value", nullptr, nullptr),
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "id_0"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_1"}, {"float_value", VALUE_IN_FILTER}},
       {{"primaryKey", "id_2"}, {"float_value", VALUE_BELOW_FILTER}},
       {{"primaryKey", "id_3"}, {"float_value", VALUE_ABOVE_FILTER}}}
   )
};

}  // namespace

QUERY_TEST(
   FloatEqualsTest,
   TEST_DATA,
   ::testing::Values(
      FLOAT_EQUALS_VALUE_SCENARIO,
      FLOAT_EQUALS_NULL_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_AND_TO_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_SCENARIO,
      FLOAT_BETWEEN_WITH_TO_SCENARIO,
      FLOAT_BETWEEN_WITH_FROM_AND_TO_NULL_SCENARIO
   )
);
