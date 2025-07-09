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
   const nlohmann::json::array_t nucleotideInsertionsSegment1,
   const nlohmann::json::array_t& nucleotideInsertionsSegment2
) {
   return {
      {"primaryKey", primaryKey},
      {"segment1", {{"seq", ""}, {"insertions", nucleotideInsertionsSegment1}}},
      {"segment2", {{"seq", ""}, {"insertions", nucleotideInsertionsSegment2}}},
      {"unaligned_segment1", nullptr},
      {"unaligned_segment2", nullptr},
      {"gene1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithNucleotideInsertions("id_0", {"123:A"}, {}),
   createDataWithNucleotideInsertions("id_1", {"123:A"}, {}),
   createDataWithNucleotideInsertions("id_2", {"234:TT"}, {}),
   createDataWithNucleotideInsertions("id_3", {"123:CCC"}, {}),
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
