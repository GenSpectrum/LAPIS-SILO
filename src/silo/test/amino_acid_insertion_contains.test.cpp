#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
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

nlohmann::json createAminoAcidInsertionContainsQuery(
   const nlohmann::json& sequenceName,
   int position,
   const std::string& insertedSymbols
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {{"type", "AminoAcidInsertionContains"},
        {"position", position},
        {"value", insertedSymbols},
        {"sequenceName", sequenceName}}}
   };
}

nlohmann::json createAminoAcidInsertionContainsQueryWithEmptySequenceName(
   int position,
   const std::string& insertedSymbols
) {
   return {
      {"action", {{"type", "Details"}}},
      {"filterExpression",
       {
          {"type", "AminoAcidInsertionContains"},
          {"position", position},
          {"value", insertedSymbols},
       }}
   };
}

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_SCENARIO = {
   .name = "aminoAcidInsertionContains",
   .query = createAminoAcidInsertionContainsQuery("gene1", 12, "A"),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO = {
   .name = "aminoAcidInsertionWithNullSegment",
   .query = createAminoAcidInsertionContainsQueryWithEmptySequenceName(12, "A"),
   .expected_error_message = "The database has no default amino acid sequence name",
};

}  // namespace

QUERY_TEST(
   AminoAcidInsertionContainsTest,
   TEST_DATA,
   ::testing::Values(
      AMINO_ACID_INSERTION_CONTAINS_SCENARIO,
      AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO
   )
);
