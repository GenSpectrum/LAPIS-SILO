#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
const std::string VALUE_SEGMENT_1 = "A";
const std::string VALUE_SEGMENT_2 = "C";

const nlohmann::json DATA_DIFFERENT_FROM_REFERENCE = {
   {"metadata", {{"primaryKey", "id"}}},
   {"alignedNucleotideSequences", {{"segment1", VALUE_SEGMENT_1}, {"segment2", VALUE_SEGMENT_2}}},
   {"unalignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
   {"alignedAminoAcidSequences",
    {{"gene1", VALUE_SEGMENT_1 + "*"}, {"gene2", VALUE_SEGMENT_2 + "*"}}},
   {"nucleotideInsertions", {{"segment1", {"1:AAA"}}, {"segment2", {"1:GGG"}}}},
   {"aminoAcidInsertions", {{"gene1", {"1:AAA"}}, {"gene2", {"1:GGG"}}}}
};

const nlohmann::json DATA_EQUALS_TO_REFERENCE = {
   {"metadata", {{"primaryKey", "equal to reference"}}},
   {"alignedNucleotideSequences", {{"segment1", "T"}, {"segment2", "T"}}},
   {"unalignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
   {"alignedAminoAcidSequences", {{"gene1", "T*"}, {"gene2", "T*"}}},
   {"nucleotideInsertions", {{"segment1", {}}, {"segment2", {}}}},
   {"aminoAcidInsertions", {{"gene1", {}}, {"gene2", {}}}}
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
   .lineage_tree = silo::common::LineageTreeAndIdMap()
};

nlohmann::json createQueryWithFilter(const nlohmann::json filter) {
   return {{"action", {{"type", "Details"}}}, {"filterExpression", filter}};
}

const nlohmann::json EXPECTED_RESULT = {{{"primaryKey", "id"}}};

const QueryTestScenario NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME = {
   .name = "nucleotideEqualsWithoutSegmentTakesDefaultSequence",
   .query = createQueryWithFilter(
      {{"type", "NucleotideEquals"}, {"position", 1}, {"symbol", VALUE_SEGMENT_1}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE = {
   .name = "nucleotideEqualsWithoutSegmentFilterByWrongValue",
   .query = createQueryWithFilter(
      {{"type", "NucleotideEquals"}, {"position", 1}, {"symbol", VALUE_SEGMENT_2}}
   ),
   .expected_query_result = nlohmann::json::array()
};

const QueryTestScenario NUCLEOTIDE_EQUALS_SEGMENT_1 = {
   .name = "nucleotideEqualsSegment1",
   .query = createQueryWithFilter(
      {{"type", "NucleotideEquals"},
       {"sequenceName", "segment1"},
       {"position", 1},
       {"symbol", VALUE_SEGMENT_1}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_EQUALS_SEGMENT_2 = {
   .name = "nucleotideEqualsSegment2",
   .query = createQueryWithFilter(
      {{"type", "NucleotideEquals"},
       {"sequenceName", "segment2"},
       {"position", 1},
       {"symbol", VALUE_SEGMENT_2}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_NO_SEQUENCE_NAME = {
   .name = "aminoAcidEqualsWithoutSequenceNameTakesDefaultSequence",
   .query = createQueryWithFilter(
      {{"type", "AminoAcidEquals"}, {"position", 1}, {"symbol", VALUE_SEGMENT_1}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_NO_SEQUENCE_NAME_FILTER_BY_WRONG_VALUE = {
   .name = "aminoAcidEqualsWithoutSequenceNameFilterByWrongValue",
   .query = createQueryWithFilter(
      {{"type", "AminoAcidEquals"}, {"position", 1}, {"symbol", VALUE_SEGMENT_2}}
   ),
   .expected_query_result = nlohmann::json::array()
};

const QueryTestScenario AMINO_ACID_EQUALS_GENE_1 = {
   .name = "aminoAcidEqualsGene1",
   .query = createQueryWithFilter(
      {{"type", "AminoAcidEquals"},
       {"sequenceName", "gene1"},
       {"position", 1},
       {"symbol", VALUE_SEGMENT_1}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_EQUALS_GENE_2 = {
   .name = "aminoAcidEqualsGene2",
   .query = createQueryWithFilter(
      {{"type", "AminoAcidEquals"},
       {"sequenceName", "gene2"},
       {"position", 1},
       {"symbol", VALUE_SEGMENT_2}}
   ),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_WITHOUT_SEQUENCE_NAME = {
   .name = "hasNucleotideMutationWithoutSequenceName",
   .query = createQueryWithFilter({{"type", "HasNucleotideMutation"}, {"position", 1}}),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario HAS_AMINO_ACID_MUTATION_WITHOUT_SEQUENCE_NAME = {
   .name = "hasAminoAcidMutationWithoutSequenceName",
   .query = createQueryWithFilter({{"type", "HasAminoAcidMutation"}, {"position", 1}}),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario NUCLEOTIDE_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME = {
   .name = "nucleotideInsertionContainsWithoutSequenceName",
   .query = createQueryWithFilter({{"type", "InsertionContains"}, {"value", "A"}, {"position", 1}}),
   .expected_query_result = EXPECTED_RESULT
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_WITHOUT_SEQUENCE_NAME = {
   .name = "aminoAcidInsertionContainsWithoutSequenceName",
   .query = createQueryWithFilter(
      {{"type", "AminoAcidInsertionContains"}, {"value", "A"}, {"position", 1}}
   ),
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
