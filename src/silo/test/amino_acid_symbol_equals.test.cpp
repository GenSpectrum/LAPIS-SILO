#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const std::string GENE = "gene1";

nlohmann::json createDataWithAminoAcidSequence(const std::string& aminoAcidSequence) {
   return {
      {"metadata", {{"primaryKey", "id_" + aminoAcidSequence}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{GENE, aminoAcidSequence}}},
      {"nucleotideInsertions", {{"segment1", {}}}},
      {"aminoAcidInsertions", {{GENE, {}}}}
   };
}
const nlohmann::json DATA_WITH_D = createDataWithAminoAcidSequence("D*");
const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_WITH_B = createDataWithAminoAcidSequence("B*");

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata = {{.name = "primaryKey", .type = ValueType::STRING}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{GENE, "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA_WITH_D, DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE, DATA_WITH_B},
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
   .name = "aminoAcidEqualsD",
   .query = createAminoAcidSymbolEqualsQuery("D", 1, GENE),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE = {
   .name = "aminoAcidEqualsM",
   .query = createAminoAcidSymbolEqualsQuery(".", 1, GENE),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

QUERY_TEST(
   AminoAcidSymbolEquals,
   TEST_DATA,
   ::testing::Values(AMINO_ACID_EQUALS_D, AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE)
);
