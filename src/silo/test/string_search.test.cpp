#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {

const std::string TEST_COLUMN = "test_column";
const std::string INDEXED_TEST_COLUMN = "indexed_test_column";

nlohmann::json createDataEntry(std::string primary_key, nlohmann::json test_column_value) {
   return {
      {"metadata",
       {{"primaryKey", primary_key},
        {TEST_COLUMN, test_column_value},
        {INDEXED_TEST_COLUMN, test_column_value}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataEntry("id1", "AA"),
   createDataEntry("id2", "BAA"),
   createDataEntry("id3", "AAB"),
   createDataEntry("id4", "ABA"),
   createDataEntry("id5", "AA"),
   createDataEntry("id6", "something else"),
   createDataEntry("id7", nullptr)
};

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata =
          {{.name = "primaryKey", .type = ValueType::STRING},
           {.name = TEST_COLUMN, .type = ValueType::STRING},
           {.name = INDEXED_TEST_COLUMN, .type = ValueType::STRING, .generate_index = true}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createStringSearchQuery(const std::string& column, const nlohmann::json value) {
   return {
      {"action", {{"type", "Details"}, {"fields", {"primaryKey"}}}},
      {"filterExpression",
       {{"type", "StringSearch"}, {"column", column}, {"searchExpression", value}}}
   };
}

nlohmann::json createExpectedResult(const std::vector<std::string>& primary_keys) {
   nlohmann::json result = nlohmann::json::array();
   for (const auto& primary_key : primary_keys) {
      result.push_back({{"primaryKey", primary_key}});
   }
   return result;
}

const QueryTestScenario FILTER_FOR_AA = {
   .name = "filterForAA",
   .query = createStringSearchQuery(TEST_COLUMN, "AA"),
   .expected_query_result = createExpectedResult({"id1", "id2", "id3", "id5"})
};

const QueryTestScenario FILTER_FOR_AA_AT_THE_BEGINNING = {
   .name = "filterForAAatTheBeginning",
   .query = createStringSearchQuery(TEST_COLUMN, "^AA"),
   .expected_query_result = createExpectedResult({"id1", "id3", "id5"})
};

const QueryTestScenario FILTER_FOR_SOMETHING_THAT_DOES_NOT_OCCUR = {
   .name = "filterForSomethingThatDoesNotOccur",
   .query = createStringSearchQuery(TEST_COLUMN, "should not match on anything"),
   .expected_query_result = createExpectedResult({})
};

const QueryTestScenario FILTER_FOR_AA_ON_INDEXED_COLUMN = {
   .name = "filterForAAOnIndexedColumn",
   .query = createStringSearchQuery(INDEXED_TEST_COLUMN, "AA"),
   .expected_query_result = createExpectedResult({"id1", "id2", "id3", "id5"})
};

const QueryTestScenario FILTER_FOR_AA_AT_THE_BEGINNING_ON_INDEXED_COLUMN = {
   .name = "filterForAAatTheBeginningOnIndexedColumn",
   .query = createStringSearchQuery(INDEXED_TEST_COLUMN, "^AA"),
   .expected_query_result = createExpectedResult({"id1", "id3", "id5"})
};

const QueryTestScenario FILTER_FOR_SOMETHING_THAT_DOES_NOT_OCCUR_ON_INDEXED_COLUMN = {
   .name = "filterForSomethingThatDoesNotOccurOnIndexedColumn",
   .query = createStringSearchQuery(INDEXED_TEST_COLUMN, "should not match on anything"),
   .expected_query_result = createExpectedResult({})
};

const QueryTestScenario INVALID_REGULAR_EXPRESSION = {
   .name = "invalidRegularExpressionShouldReturnProperError",
   .query = createStringSearchQuery(TEST_COLUMN, "^("),
   .expected_error_message =
      "Invalid Regular Expression. The parsing of the regular expression failed with the error "
      "'missing ): ^('. See https://github.com/google/re2/wiki/Syntax for a Syntax specification."
};

const QueryTestScenario FILTER_FOR_NULL_IS_NOT_POSSIBLE = {
   .name = "filterForNullIsNotPossible",
   .query = createStringSearchQuery(TEST_COLUMN, nullptr),
   .expected_error_message =
      "The field 'searchExpression' in an StringSearch expression needs to be a string"
};

const QueryTestScenario FILTER_FOR_COLUMN_THAT_DOES_NOT_EXIST = {
   .name = "filterForColumnThatDoesNotExist",
   .query = createStringSearchQuery("column_that_does_not_exist", "some value"),
   .expected_error_message =
      "The database does not contain the string column 'column_that_does_not_exist'"
};

}  // namespace

QUERY_TEST(
   StringSearchTest,
   TEST_DATA,
   ::testing::Values(
      FILTER_FOR_AA,
      FILTER_FOR_AA_AT_THE_BEGINNING,
      FILTER_FOR_SOMETHING_THAT_DOES_NOT_OCCUR,
      FILTER_FOR_AA_ON_INDEXED_COLUMN,
      FILTER_FOR_AA_AT_THE_BEGINNING_ON_INDEXED_COLUMN,
      FILTER_FOR_SOMETHING_THAT_DOES_NOT_OCCUR_ON_INDEXED_COLUMN,
      INVALID_REGULAR_EXPRESSION,
      FILTER_FOR_NULL_IS_NOT_POSSIBLE,
      FILTER_FOR_COLUMN_THAT_DOES_NOT_EXIST
   )
);
