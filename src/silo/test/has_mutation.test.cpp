#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createDataWithSequences(
   const std::string& nucleotideSequence,
   const std::string& aminoAcidSequence
) {
   random_generator generator;
   const auto primary_key = generator();
   return {
      {"primaryKey", "id_" + to_string(primary_key)},
      {"unaligned_segment1", {}},
      {"segment1", {{"seq", nucleotideSequence}, {"insertions", nlohmann::json::array()}}},
      {"gene1", {{"seq", aminoAcidSequence}, {"insertions", nlohmann::json::array()}}}
   };
}

const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithSequences("ATGCN", "M*");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithSequences("ATGCN", "C*");
const nlohmann::json DATA_WITH_ALL_N = createDataWithSequences("NNNNN", "M*");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithSequences("CATTT", "X*");

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
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_ALL_N, DATA_WITH_ALL_MUTATED},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createHasNucleotideMutationQuery(int position) {
   return {
      {"action", {{"type", "Aggregated"}}},
      {"filterExpression",
       {{"type", "HasNucleotideMutation"}, {"position", position}, {"sequenceNames", {"segment1"}}}}
   };
}

nlohmann::json createHasAminoAcidMutationQuery(int position) {
   return {
      {"action", {{"type", "Aggregated"}}},
      {"filterExpression",
       {{"type", "HasAminoAcidMutation"}, {"position", position}, {"sequenceNames", {"gene1"}}}}
   };
}

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION = {
   .name = "HAS_NUCLEOTIDE_MUTATION",
   .query = createHasNucleotideMutationQuery(1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario HAS_AMINO_ACID_MUTATION = {
   .name = "HAS_AMINO_ACID_MUTATION",
   .query = createHasAminoAcidMutationQuery(1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE = {
   .name = "HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE",
   .query = createHasNucleotideMutationQuery(2000),
   .expected_error_message = "HasNucleotideMutation position is out of bounds 2000 > 5"
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_LOW = {
   .name = "HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_HIGH",
   .query = createHasNucleotideMutationQuery(0),
   .expected_error_message = "The field 'position' is 1-indexed. Value of 0 not allowed."
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_HIGH = {
   .name = "HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_LOW",
   .query = createHasNucleotideMutationQuery(6),
   .expected_error_message = "HasNucleotideMutation position is out of bounds 6 > 5"
};

const QueryTestScenario HAS_NUCLEOTIDE_MUTATION_IN_RANGE_EDGE = {
   .name = "HAS_NUCLEOTIDE_MUTATION_IN_RANGE_EDGE",
   .query = createHasNucleotideMutationQuery(5),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario HAS_AMINO_ACID_MUTATION_OUT_OF_RANGE = {
   .name = "HAS_AMINO_ACID_MUTATION_OUT_OF_RANGE",
   .query = createHasAminoAcidMutationQuery(1000),
   .expected_error_message = "HasAminoAcidMutation position is out of bounds 1000 > 2"
};

}  // namespace

QUERY_TEST(
   HasMutation,
   TEST_DATA,
   ::testing::Values(
      HAS_NUCLEOTIDE_MUTATION,
      HAS_AMINO_ACID_MUTATION,
      HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE,
      HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_LOW,
      HAS_NUCLEOTIDE_MUTATION_OUT_OF_RANGE_EDGE_HIGH,
      HAS_NUCLEOTIDE_MUTATION_IN_RANGE_EDGE,
      HAS_AMINO_ACID_MUTATION_OUT_OF_RANGE
   )
);
