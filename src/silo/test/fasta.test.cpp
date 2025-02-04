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
   const std::string& date,
   const nlohmann::json& segment1,
   const nlohmann::json& segment2
) {
   return {
      {"metadata", {{"primaryKey", primaryKey}, {"date", date}}},
      {"alignedNucleotideSequences", {{"segment1", nullptr}, {"segment2", nullptr}}},
      {"unalignedNucleotideSequences", {{"segment1", segment1}, {"segment2", segment2}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}, {"gene2", nullptr}}},
      {"nucleotideInsertions", {{"segment1", {}}, {"segment2", {}}}},
      {"aminoAcidInsertions", {{"gene1", {}}, {"gene2", {}}}}
   };
}

const std::vector<nlohmann::json> DATA = {
   createDataWithUnalignedSequences("bothSegments", "2024-08-01", "A", "G"),
   createDataWithUnalignedSequences("onlySegment1", "2024-08-03", "T", nullptr),
   createDataWithUnalignedSequences("onlySegment2", "2024-08-02", nullptr, "T"),
   createDataWithUnalignedSequences("noSegment", "2024-08-08", nullptr, nullptr),
   createDataWithUnalignedSequences("1", "2024-08-05", nullptr, "A"),
   createDataWithUnalignedSequences("2", "2024-08-03", nullptr, nullptr),
   createDataWithUnalignedSequences("3", "2024-08-02", nullptr, "AA")
};

const auto DATABASE_CONFIG = silo::config::DatabaseConfigReader().parseYaml(
   R"(
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "date"
      type: "date"
  primaryKey: "primaryKey"
  dateToSortBy: "date"
)"
);

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

const QueryTestScenario DOWNLOAD_ALL_SEQUENCES_SCENARIO = {
   .name = "downloadAllSequences",
   .query =
      {{"action",
        {{"type", "Fasta"},
         {"orderByFields", {"primaryKey"}},
         {"sequenceName", {"segment1", "segment2"}}}},
       {"filterExpression", {{"type", "True"}}}},
   .expected_query_result = nlohmann::json(
      {{{"primaryKey", "1"}, {"segment1", nullptr}, {"segment2", "A"}},
       {{"primaryKey", "2"}, {"segment1", nullptr}, {"segment2", nullptr}},
       {{"primaryKey", "3"}, {"segment1", nullptr}, {"segment2", "AA"}},
       {{"primaryKey", "bothSegments"}, {"segment1", "A"}, {"segment2", "G"}},
       {{"primaryKey", "noSegment"}, {"segment1", nullptr}, {"segment2", nullptr}},
       {{"primaryKey", "onlySegment1"}, {"segment1", "T"}, {"segment2", nullptr}},
       {{"primaryKey", "onlySegment2"}, {"segment1", nullptr}, {"segment2", "T"}}}
   )
};

}  // namespace

QUERY_TEST(
   FastaTest,
   TEST_DATA,
   ::testing::Values(
      SEQUENCE_WITH_BOTH_SEGMENTS_SCENARIO,
      SEQUENCE_WITH_ONLY_FIRST_SEGMENT_SCENARIO,
      SEQUENCE_WITH_ONLY_SECOND_SEGMENT_SCENARIO,
      SEQUENCE_WITH_NO_SEGMENT_SCENARIO,
      DOWNLOAD_ALL_SEQUENCES_SCENARIO
   )
);
