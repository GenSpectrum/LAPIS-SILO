#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {

const std::string GENE = "gene1";

size_t idx = 0;

nlohmann::json createDataWithAminoAcidSequence(const std::string& aminoAcidSequence) {
   return {
      {"primaryKey", fmt::format("id_{}", idx++)},
      {"segment1", nullptr},
      {GENE, {{"sequence", aminoAcidSequence}, {"insertions", nlohmann::json::array()}}},
      {"unaligned_segment1", {}}
   };
}
const nlohmann::json DATA_WITH_D = createDataWithAminoAcidSequence("D*");
const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_WITH_B = createDataWithAminoAcidSequence("B*");

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
   {{"segment1", "A"}},
   {{GENE, "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA_WITH_D, DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_B},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createAminoAcidSymbolEqualsQuery(
   const std::string& symbol,
   int position,
   const std::string& gene
) {
   return {
      {"action", {{"type", "Aggregated"}}},
      {"filterExpression",
       {{"type", "AminoAcidEquals"},
        {"position", position},
        {"symbol", symbol},
        {"sequenceName", gene}}}
   };
}

const QueryTestScenario AMINO_ACID_EQUALS_D = {
   .name = "AMINO_ACID_EQUALS_D",
   .query = createAminoAcidSymbolEqualsQuery("D", 1, GENE),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE = {
   .name = "AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE",
   .query = createAminoAcidSymbolEqualsQuery(".", 1, GENE),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

}  // namespace

QUERY_TEST(
   AminoAcidSymbolEquals,
   TEST_DATA,
   ::testing::Values(AMINO_ACID_EQUALS_D, AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE)
);
