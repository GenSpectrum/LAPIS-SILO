#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace nucleotide {

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
   createDataWithNucleotideInsertions("id_4", {"0:A"}, {}),
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

const QueryTestScenario INSERTION_CONTAINS_SCENARIO_POSITION_0_EQUALS_BEFORE_FIRST = {
   .name = "INSERTION_CONTAINS_SCENARIO_POSITION_0_EQUALS_BEFORE_FIRST",
   .query =
      "default.filter(insertionContains(position:=0, value:='A', "
      "sequenceName:='segment1')).project(primaryKey)",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_4"}}})
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
      "default.filter(insertionContains(position:=100, value:='A', sequenceName:='segment2'))",
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

}  // namespace nucleotide

namespace amino_acid {

nlohmann::json createDataWithAminoAcidInsertions(
   const std::string& primaryKey,
   const nlohmann::json::array_t& aminoAcidInsertionsGene1,
   const nlohmann::json::array_t& aminoAcidInsertionsGene2
) {
   return {
      {"primaryKey", primaryKey},
      {"segment1", nullptr},
      {"segment2", nullptr},
      {"unaligned_segment1", nullptr},
      {"unaligned_segment2", nullptr},
      {"gene1", {{"sequence", "ABCDEFGHIKLMNPQRSTVWYZ*"}, {"insertions", aminoAcidInsertionsGene1}}
      },
      {"gene2", {{"sequence", "ABCDEFGHIKLMNPQRSTVWYZ*"}, {"insertions", aminoAcidInsertionsGene2}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithAminoAcidInsertions("id_0", {"12:A"}, nlohmann::json::array()),
   createDataWithAminoAcidInsertions("id_1", {"12:A"}, nlohmann::json::array()),
   createDataWithAminoAcidInsertions("id_2", {"23:BB"}, nlohmann::json::array()),
   createDataWithAminoAcidInsertions("id_3", {"12:CCC"}, nlohmann::json::array()),
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
   {{"gene1", "ABCDEFGHIKLMNPQRSTVWYZ*"}, {"gene2", "ABCDEFGHIKLMNPQRSTVWYZ*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATA,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_SCENARIO = {
   .name = "AMINO_ACID_INSERTION_CONTAINS_SCENARIO",
   .query =
      "default.filter(aminoAcidInsertionContains(position:=12, value:='A', "
      "sequenceName:='gene1')).project(primaryKey)",
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO = {
   .name = "AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO",
   .query = "default.filter(aminoAcidInsertionContains(position:=12, value:='A'))",
   .expected_error_message = "The database has no default amino acid sequence name",
};

}  // namespace amino_acid

}  // namespace

QUERY_TEST(
   InsertionContainsTest,
   nucleotide::TEST_DATA,
   ::testing::Values(
      nucleotide::INSERTION_CONTAINS_SCENARIO,
      nucleotide::INSERTION_CONTAINS_SCENARIO_POSITION_0_EQUALS_BEFORE_FIRST,
      nucleotide::INSERTION_CONTAINS_WITH_EMPTY_SEGMENT_SCENARIO,
      nucleotide::INSERTION_CONTAINS_WITH_UNKNOWN_SEGMENT_SCENARIO,
      nucleotide::INSERTION_CONTAINS_POSITION_OUT_OF_RANGE,
      nucleotide::INSERTION_CONTAINS_POSITION_OUT_OF_RANGE_DEFAULT_SEQUENCE
   )
);

QUERY_TEST(
   AminoAcidInsertionContainsTest,
   amino_acid::TEST_DATA,
   ::testing::Values(
      amino_acid::AMINO_ACID_INSERTION_CONTAINS_SCENARIO,
      amino_acid::AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO
   )
);
