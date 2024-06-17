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
   const nlohmann::json& aminoAcidInsertions
) {
   return {
      {"metadata", {{"primaryKey", primaryKey}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}, {"gene2", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}, {"segment2", {}}}},
      {"aminoAcidInsertions", aminoAcidInsertions}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithAminoAcidInsertions("id_0", {{"gene1", {"123:A"}}, {"gene2", {}}}),
   createDataWithAminoAcidInsertions("id_1", {{"gene1", {"123:A"}}, {"gene2", {}}}),
   createDataWithAminoAcidInsertions("id_2", {{"gene1", {"234:BB"}}, {"gene2", {}}}),
   createDataWithAminoAcidInsertions("id_3", {{"gene1", {"123:CCC"}}, {"gene2", {}}}),
};

const auto DATABASE_CONFIG = DatabaseConfig{
   .default_nucleotide_sequence = "segment1",
   .schema =
      {.instance_name = "dummy name",
       .metadata = {{.name = "primaryKey", .type = ValueType::STRING}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}, {"segment2", "T"}},
   {{"gene1", "*"}, {"gene2", "*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA},
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
   .query = createAminoAcidInsertionContainsQuery("gene1", 123, "A"),
   .expected_query_result = nlohmann::json({{{"primaryKey", "id_0"}}, {{"primaryKey", "id_1"}}})
};

const QueryTestScenario AMINO_ACID_INSERTION_CONTAINS_WITH_NULL_SEGMENT_SCENARIO = {
   .name = "aminoAcidInsertionWithNullSegment",
   .query = createAminoAcidInsertionContainsQueryWithEmptySequenceName(123, "A"),
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
