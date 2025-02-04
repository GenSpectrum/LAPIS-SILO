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

nlohmann::json createDataWithNucleotideSequence(const std::string& nucleotideSequence) {
   random_generator generator;
   const auto primary_key = generator();

   return {
      {"metadata", {{"primaryKey", "id_" + to_string(primary_key)}}},
      {"alignedNucleotideSequences", {{"segment1", nucleotideSequence}}},
      {"unalignedNucleotideSequences", {{"segment1", {}}}},
      {"alignedAminoAcidSequences", {{"gene1", {}}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}}}
   };
}

const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_WITH_ALL_N = createDataWithNucleotideSequence("NNNNN");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithNucleotideSequence("CATTT");

const auto DATABASE_CONFIG = silo::config::DatabaseConfigReader().parseYaml(
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)"
);

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

nlohmann::json createNucleotideSymbolEqualsQuery(const std::string& symbol, int position) {
   return {
      {"action", {{"type", "Aggregated"}}},
      {"filterExpression",
       {{"type", "NucleotideEquals"},
        {"position", position},
        {"symbol", symbol},
        {"sequenceName", "segment1"}}}
   };
}

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL = {
   .name = "nucleotideEqualsWithSymbol",
   .query = createNucleotideSymbolEqualsQuery("C", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE = {
   .name = "nucleotideEqualsWithDot",
   .query = createNucleotideSymbolEqualsQuery(".", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

}  // namespace

QUERY_TEST(
   NucleotideSymbolEquals,
   TEST_DATA,
   ::testing::Values(NUCLEOTIDE_EQUALS_WITH_SYMBOL, NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE)
);
