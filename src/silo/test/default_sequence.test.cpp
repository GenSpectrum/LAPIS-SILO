#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
const std::string VALUE_SEGMENT_1 = "A";
const std::string VALUE_SEGMENT_2 = "C";

const nlohmann::json DATA_DIFFERENT_FROM_REFERENCE = {
   {"primaryKey", "id"},
   {"segment1", {{"sequence", VALUE_SEGMENT_1}, {"insertions", {"1:AAA"}}}},
   {"segment2", {{"sequence", VALUE_SEGMENT_2}, {"insertions", {"1:GGG"}}}},
   {"unaligned_segment1", nullptr},
   {"unaligned_segment2", nullptr},
   {"gene1", {{"sequence", VALUE_SEGMENT_1 + "*"}, {"insertions", {"1:AAA"}}}},
   {"gene2", {{"sequence", VALUE_SEGMENT_2 + "*"}, {"insertions", {"1:GGG"}}}},
};

const nlohmann::json DATA_EQUALS_TO_REFERENCE = {
   {"primaryKey", "equal to reference"},
   {"segment1", {{"sequence", "T"}, {"insertions", nlohmann::json::array()}}},
   {"segment2", {{"sequence", "T"}, {"insertions", nlohmann::json::array()}}},
   {"gene1", {{"sequence", "T*"}, {"insertions", nlohmann::json::array()}}},
   {"gene2", {{"sequence", "T*"}, {"insertions", nlohmann::json::array()}}},
   {"unaligned_segment1", nullptr},
   {"unaligned_segment2", nullptr},
};

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
defaultAminoAcidSequence: "gene1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "T"}, {"segment2", "T"}},
   {{"gene1", "T*"}, {"gene2", "T*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA_DIFFERENT_FROM_REFERENCE, DATA_EQUALS_TO_REFERENCE},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .lineage_trees = {{"test", silo::common::LineageTreeAndIdMap()}}
};

const nlohmann::json EXPECTED_RESULT = {{{"primaryKey", "id"}}};

const QueryTestScenario NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME = {
   .name = "nucleotideEqualsWithoutSegmentTakesDefaultSequence",
   .query = "default.filter(nucleotideEquals(position:=1, symbol:='A')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE = {
   .name = "nucleotideEqualsWithoutSegmentFilterByWrongValue",
   .query = "default.filter(nucleotideEquals(position:=1, symbol:='C')).project(primaryKey)",
   .expected_query_result = nlohmann::json::array()
};

const QueryTestScenario NUCLEOTIDE_EQUALS_SEGMENT_1 = {
   .name = "nucleotideEqualsSegment1",
   .query =
      "default.filter(nucleotideEquals(position:=1, symbol:='A', "
      "sequenceName:='segment1')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_EQUALS_SEGMENT_2 = {
   .name = "nucleotideEqualsSegment2",
   .query =
      "default.filter(nucleotideEquals(position:=1, symbol:='C', "
      "sequenceName:='segment2')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_NO_SEQUENCE_NAME = {
   .name = "aminoAcidEqualsWithoutSequenceNameTakesDefaultSequence",
   .query = "default.filter(aminoAcidEquals(position:=1, symbol:='A')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE = {
   .name = "aminoAcidEqualsWithoutSequenceNameFilterByWrongValue",
   .query = "default.filter(aminoAcidEquals(position:=1, symbol:='C')).project(primaryKey)",
   .expected_query_result = nlohmann::json::array()
};

const QueryTestScenario AMINO_ACID_EQUALS_GENE_1 = {
   .name = "aminoAcidEqualsGene1",
   .query =
      "default.filter(aminoAcidEquals(position:=1, symbol:='A', "
      "sequenceName:='gene1')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_GENE_2 = {
   .name = "aminoAcidEqualsGene2",
   .query =
      "default.filter(aminoAcidEquals(position:=1, symbol:='C', "
      "sequenceName:='gene2')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_WITHOUT_SEQUENCE_NAME = {
   .name = "hasNucleotideMutationWithoutSequenceName",
   .query = "default.filter(hasMutation(position:=1)).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario HAS_AMINO_ACID_MUTATION_WITHOUT_SEQUENCE_NAME = {
   .name = "hasAminoAcidMutationWithoutSequenceName",
   .query = "default.filter(hasAAMutation(position:=1)).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME = {
   .name = "nucleotideInsertionContainsWithoutSequenceName",
   .query = "default.filter(insertionContains(position:=1, value:='AAA')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME = {
   .name = "aminoAcidInsertionContainsWithoutSequenceName",
   .query =
      "default.filter(aminoAcidInsertionContains(position:=1, value:='AAA')).project(primaryKey)",
   .expected_query_result = EXPECTED_RESULT
};

}  // namespace

QUERY_TEST(
   DefaultSequenceTest,
   TEST_DATA,
   ::testing::Values(
      NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME,
      NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE,
      NUCLEOTIDE_EQUALS_SEGMENT_1,
      NUCLEOTIDE_EQUALS_SEGMENT_2,
      AMINO_ACID_EQUALS_NO_SEQUENCE_NAME,
      AMINO_ACID_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE,
      AMINO_ACID_EQUALS_GENE_1,
      AMINO_ACID_EQUALS_GENE_2,
      HAS_NUCLEOTIDE_MUTATION_WITHOUT_SEQUENCE_NAME,
      HAS_AMINO_ACID_MUTATION_WITHOUT_SEQUENCE_NAME,
      NUCLEOTIDE_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME,
      AMINO_ACID_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME
   )
);
