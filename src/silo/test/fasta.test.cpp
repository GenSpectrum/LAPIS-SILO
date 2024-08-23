#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace {
nlohmann::json createDataWithUnalignedSequences(
   const std::string& primaryKey,
   const nlohmann::json& segment1,
   const nlohmann::json& segment2
) {
   return {
      {"metadata", {{"primaryKey", primaryKey}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", segment1}, {"segment2", segment2}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}, {"gene2", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}, {"segment2", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}, {"gene2", {}}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithUnalignedSequences("bothSegments", "A", "G"),
   createDataWithUnalignedSequences("onlySegment1", "T", nullptr),
   createDataWithUnalignedSequences("onlySegment2", nullptr, "T"),
   createDataWithUnalignedSequences("noSegment", nullptr, nullptr)
};

const auto DATABASE_CONFIG = DatabaseConfig{
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

nlohmann::json createFastaAlignedQuery(const std::string& primaryKey) {
   return {
      {"action", {{"type", "Fasta"}, {"sequenceName", {"segment1", "segment2"}}}},
      {"filterExpression",
       {
          {"type", "StringEquals"},
          {"column", "primaryKey"},
          {"value", primaryKey},
       }}
   };
}

const QueryTestScenario SEQUENCE_WITH_BOTH_SEGMENTS_SCENARIO = {
   .name = "sequenceWithBothSegments",
   .query = createFastaAlignedQuery("bothSegments"),
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "bothSegments"}, {"segment1", "A"}, {"segment2", "G"}}})
};

const QueryTestScenario SEQUENCE_WITH_ONLY_FIRST_SEGMENT_SCENARIO = {
   .name = "sequenceWithOnlyFirstSegment",
   .query = createFastaAlignedQuery("onlySegment1"),
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "onlySegment1"}, {"segment1", "T"}, {"segment2", nullptr}}})
};

const QueryTestScenario SEQUENCE_WITH_ONLY_SECOND_SEGMENT_SCENARIO = {
   .name = "sequenceWithOnlySecondSegment",
   .query = createFastaAlignedQuery("onlySegment2"),
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "onlySegment2"}, {"segment1", nullptr}, {"segment2", "T"}}})
};

const QueryTestScenario SEQUENCE_WITH_NO_SEGMENT_SCENARIO = {
   .name = "sequenceWithNoSegment",
   .query = createFastaAlignedQuery("noSegment"),
   .expected_query_result =
      nlohmann::json({{{"primaryKey", "noSegment"}, {"segment1", nullptr}, {"segment2", nullptr}}})
};

}  // namespace

QUERY_TEST(
   FastaTest,
   TEST_DATA,
   ::testing::Values(
      SEQUENCE_WITH_BOTH_SEGMENTS_SCENARIO,
      SEQUENCE_WITH_ONLY_FIRST_SEGMENT_SCENARIO,
      SEQUENCE_WITH_ONLY_SECOND_SEGMENT_SCENARIO,
      SEQUENCE_WITH_NO_SEGMENT_SCENARIO
   )
);
