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
   {{"segment1", "ATGCN"}},
   {},
};

nlohmann::json segmentValue(const std::string& nucleotide_sequence) {
   return {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}};
}

constexpr std::array GROUP_SYMBOLS = {'A', 'C', 'G', 'T'};

nlohmann::json createRow(size_t row_index) {
   std::string segment_value = "ATGCN";
   segment_value.at(0) = GROUP_SYMBOLS.at(row_index % GROUP_SYMBOLS.size());
   return {
      {"primaryKey", "id_" + std::to_string(row_index)},
      {"segment1", segmentValue(segment_value)},
   };
}

const QueryTestData TEST_DATA{
   .ndjson_input_data = buildLargeTestDataset(createRow, 500'000),
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true,
};

const QueryTestScenario GROUP_BY_POSITION_ACROSS_CHUNKS = {
   .name = "GROUP_BY_POSITION_ACROSS_CHUNKS",
   .query = "data.map({s1 := segment1.at(1)}).group(by:={s1}, aggs:={count:=count()})",
   .expected_query_result = nlohmann::json::array({
      {{"s1", "A"}, {"count", 125'000}},
      {{"s1", "C"}, {"count", 125'000}},
      {{"s1", "G"}, {"count", 125'000}},
      {{"s1", "T"}, {"count", 125'000}},
   }),
};
}  // namespace

QUERY_TEST(
   BitmapAggregationCrossChunk,
   TEST_DATA,
   ::testing::Values(GROUP_BY_POSITION_ACROSS_CHUNKS)
);
