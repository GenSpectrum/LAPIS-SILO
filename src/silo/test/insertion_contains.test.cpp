#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
nlohmann::json createDataWithNucleotideInsertions(
   const std::string& primaryKey,
   const nlohmann::json& nucleotideInsertions
) {
   return {
      {"metadata", {{"primaryKey", primaryKey}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}},
      {"nucleotideInsertions", nucleotideInsertions},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithNucleotideInsertions("id_0", {{"segment1", {"123:A"}}, {"segment2", {}}}),
   createDataWithNucleotideInsertions("id_1", {{"segment1", {"123:A"}}, {"segment2", {}}}),
   createDataWithNucleotideInsertions("id_2", {{"segment1", {"234:TT"}}, {"segment2", {}}}),
   createDataWithNucleotideInsertions("id_3", {{"segment1", {"123:CCC"}}, {"segment2", {}}}),
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}, {"segment2", "T"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createInsertionContainsQuery(
   const nlohmann::json& sequenceName,
   int position,
   const std::string& insertedSymbols
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "InsertionContains"},
        {"position", position},
        {"value", insertedSymbols},
        {"sequenceName", sequenceName}}}
   };
}

nlohmann::json createInsertionContainsQueryWithEmptySequenceName(
   int position,
   const std::string& insertedSymbols
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {
          {"type", "InsertionContains"},
          {"position", position},
          {"value", insertedSymbols},
       }}
   };
}

const QueryTestScenario INSERTION_CONTAINS_SCENARIO = {
   .name = "insertionContains",
   .query = createInsertionContainsQuery("segment1", 123, "A"),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO = {
   .name = "insertionContainsWithNullSegmentDefaultsToDefaultSegment",
   .query = createInsertionContainsQueryWithEmptySequenceName(123, "A"),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO = {
   .name = "insertionContainsWithUnknownSegment",
   .query = createInsertionContainsQuery("unknownSegmentName", 123, "A"),
   .expected_error_message =
      "Database does not contain the Nucleotide Sequence with name: 'unknownSegmentName'"
};
}  // namespace
QUERY_TEST(
   InsertionContainsTest,
   TEST_DATA,
   ::testing::Values(
      INSERTION_CONTAINS_SCENARIO,
      INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO,
      INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO
   )
);
