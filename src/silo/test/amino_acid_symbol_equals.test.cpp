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
const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_WITH_B = createDataWithAminoAcidSequence("B*");

const auto DATABASE_CONFIG =
   DatabaseConfig{"segment1", {"dummy name", {{"primaryKey", ValueType::STRING}}, "primaryKey"}};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{GENE, "M*"}},
};

const QueryTestData TEST_DATA{
   {DATA_WITH_D, DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE, DATA_WITH_B},
   DATABASE_CONFIG,
   REFERENCE_GENOMES
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
   "aminoAcidEqualsD",
   createAminoAcidSymbolEqualsQuery("D", 1, GENE),
   {{{"count", 1}}}
};

const QueryTestScenario AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE = {
   "aminoAcidEqualsM",
   createAminoAcidSymbolEqualsQuery(".", 1, GENE),
   {{{"count", 2}}}
};

QUERY_TEST(
   AminoAcidSymbolEquals,
   TEST_DATA,
   ::testing::Values(AMINO_ACID_EQUALS_D, AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE)
);
