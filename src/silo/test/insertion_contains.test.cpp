#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
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
      {"segment1",
       {{"sequence", "AAAACCCCGGGGTTTTAAAACCCCGGGGTTTT"},
        {"insertions", nucleotideInsertionsSegment1}}},
      {"segment2",
       {{"sequence", "CCCCGGGGTTTTAAAACCCCGGGGTTTTAAAA"},
        {"insertions", nucleotideInsertionsSegment2}}},
      {"unaligned_segment1", nullptr},
      {"unaligned_segment2", nullptr},
      {"gene1", nullptr}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithNucleotideInsertions("id_0", {"12:A"}, {}),
   createDataWithNucleotideInsertions("id_1", {"12:A"}, {}),
   createDataWithNucleotideInsertions("id_2", {"23:TT"}, {}),
   createDataWithNucleotideInsertions("id_3", {"12:CCC"}, {}),
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
   {{"segment1", "AAAACCCCGGGGTTTTAAAACCCCGGGGTTTT"},
    {"segment2", "CCCCGGGGTTTTAAAACCCCGGGGTTTTAAAA"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario INSERTION_CONTAINS_SCENARIO = {
   .name = "INSERTION_CONTAINS_SCENARIO",
   .query =
      "default.filter(insertionContains(position:=12, value:='A', "
      "sequenceName:='segment1')).project(primaryKey)",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO = {
   .name = "INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO",
   .query = "default.filter(insertionContains(position:=12, value:='A')).project(primaryKey)",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO = {
   .name = "INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO",
   .query =
      "default.filter(insertionContains(position:=12, value:='A', "
      "sequenceName:='unknownSegmentName'))",
   .expected_error_message =
      "Database does not contain the Nucleotide Sequence with name: 'unknownSegmentName'"
};

const QueryTestScenario INSERTION_CONTAINS_POSITION_OUT_OF_RANGE = {
   .name = "INSERTION_CONTAINS_POSITION_OUT_OF_RANGE",
   .query =
      "default.filter(insertionContains(position:=100, value:='A', sequenceName:='segment2'))"
      "",
   .expected_error_message =
      "the requested insertion position (100) is larger than the length of the reference sequence "
      "(32) for sequence 'segment2'"
};

const QueryTestScenario INSERTION_CONTAINS_POSITION_OUT_OF_RANGE_DEFAULT_SEQUENCE = {
   .name = "INSERTION_CONTAINS_POSITION_OUT_OF_RANGE_DEFAULT_SEQUENCE",
   .query = "default.filter(insertionContains(position:=100, value:='A'))",
   .expected_error_message =
      "the requested insertion position (100) is larger than the length of the reference sequence "
      "(32) for sequence 'segment1'"
};

}  // namespace
QUERY_TEST(
   InsertionContainsTest,
   TEST_DATA,
   ::testing::Values(
      INSERTION_CONTAINS_SCENARIO,
      INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO,
      INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO,
      INSERTION_CONTAINS_POSITION_OUT_OF_RANGE,
      INSERTION_CONTAINS_POSITION_OUT_OF_RANGE_DEFAULT_SEQUENCE
   )
);
