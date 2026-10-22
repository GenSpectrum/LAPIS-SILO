#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "rhydb/storage/reference_genomes.h"
#include "rhydb/test/large_dataset.test.h"
#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::buildLargeTestDataset;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "large dataset"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGC"}},
   {},
};

nlohmann::json segmentValue(const std::string& nucleotide_sequence) {
   return {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}};
}

constexpr std::array<const char*, 5> ROW_SEQUENCES = {
   "ATGC",
   "CTGC",
   "GTGC",
   "CTGC",
   "NTGC",
};

nlohmann::json createRow(const size_t row_index) {
   return {
      {"primaryKey", "id_" + std::to_string(row_index)},
      {"segment1", segmentValue(ROW_SEQUENCES.at(row_index % ROW_SEQUENCES.size()))},
   };
}

const QueryTestData TEST_DATA{
   .ndjson_input_data = buildLargeTestDataset(createRow, 500'000),
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true,
};

const QueryTestScenario COUNT_FILTER_ACROSS_CHUNKS = {
   .name = "COUNT_FILTER_ACROSS_CHUNKS",
   .query =
      "data.filter(nucleotideEquals(position := 1, symbol := 'C', sequenceName := 'segment1'))"
      ".group(by:={}, aggs:={count:=count()})",
   .expected_query_result = nlohmann::json::array({
      {{"count", 200'000}},
   }),
};

const QueryTestScenario COUNT_NEGATED_FILTER_ACROSS_CHUNKS = {
   .name = "COUNT_NEGATED_FILTER_ACROSS_CHUNKS",
   .query =
      "data.filter(!(nucleotideEquals(position := 1, symbol := 'C', sequenceName := 'segment1')))"
      ".group(by:={}, aggs:={count:=count()})",
   .expected_query_result = nlohmann::json::array({
      {{"count", 300'000}},
   }),
};
}  // namespace

QUERY_TEST(
   CountFilterNodeCrossChunk,
   TEST_DATA,
   ::testing::Values(COUNT_FILTER_ACROSS_CHUNKS, COUNT_NEGATED_FILTER_ACROSS_CHUNKS)
);
