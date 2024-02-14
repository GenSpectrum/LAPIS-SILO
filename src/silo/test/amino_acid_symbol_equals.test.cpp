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
      {"alignedAminoAcidSequences", {{GENE, aminoAcidSequence}}}
   };
}
const nlohmann::json DATA_WITH_D = createDataWithAminoAcidSequence("D*");
const nlohmann::json DATA_WITH_M = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_WITH_B = createDataWithAminoAcidSequence("B*");

const auto DATABASE_CONFIG =
   DatabaseConfig{"segmenet1", {"dummy name", {{"primaryKey", ValueType::STRING}}, "primaryKey"}};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   {DATA_WITH_D, DATA_WITH_M, DATA_WITH_B},
   DATABASE_CONFIG,
   REFERENCE_GENOMES
};

const nlohmann::json QUERY = {
   {"action", {{"type", "Aggregated"}}},
   {"filterExpression",
    {{"type", "AminoAcidEquals"}, {"position", 1}, {"symbol", "D"}, {"sequenceName", GENE}}}
};

const nlohmann::json EXPECTED = {{{"count", 1}}};

const QueryTestScenario AMINO_ACID_EQUALS_D = {"aminoAcidEqualsD", QUERY, EXPECTED};

QUERY_TEST(AminoAcidSymbolEquals, TEST_DATA, ::testing::Values(AMINO_ACID_EQUALS_D));
