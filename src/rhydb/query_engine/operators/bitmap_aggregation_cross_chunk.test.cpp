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

nlohmann::json createTemplate(size_t chunk_index, size_t row_index) {
   return {
      {"primaryKey", "id_c" + std::to_string(chunk_index) + "_" + std::to_string(row_index)},
      {"segment1", nullptr},
   };
}

nlohmann::json segment1Value(const std::string& nucleotide_sequence) {
   return {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}};
}

const rhydb::test::LargeDataset DATASET = buildLargeTestDataset(
   createTemplate,
   {
      {
         {{"segment1", segment1Value("ATGCN")}},
         {{"segment1", segment1Value("CTGCN")}},
         {{"segment1", segment1Value("GTGCN")}},
      },
      {
         {{"segment1", segment1Value("ATGCN")}},
         {{"segment1", segment1Value("CTGCN")}},
         {{"segment1", segment1Value("TTGCN")}},
      },
      {
         {{"segment1", segment1Value("ATGCN")}},
         {{"segment1", segment1Value("TTGCN")}},
      },
   }
);

const QueryTestData TEST_DATA{
   .ndjson_input_data = DATASET.rows,
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true,
};

// Groups come out in nucleotide SYMBOLS order (A, C, G, ...) with the null
// group (all filler rows) emitted last.
const QueryTestScenario GROUP_BY_POSITION_ACROSS_CHUNKS = {
   .name = "GROUP_BY_POSITION_ACROSS_CHUNKS",
   .query = "data.map({s1 := segment1.at(1)}).group(by:={s1}, aggs:={count:=count()})",
   .expected_query_result = nlohmann::json::array({
      {{"s1", "A"}, {"count", 3}},
      {{"s1", "C"}, {"count", 2}},
      {{"s1", "G"}, {"count", 1}},
      {{"s1", "T"}, {"count", 2}},
      {{"s1", nullptr}, {"count", static_cast<int64_t>(DATASET.filler_count)}},
   }),
};
}  // namespace

QUERY_TEST(
   BitmapAggregationCrossChunk,
   TEST_DATA,
   ::testing::Values(GROUP_BY_POSITION_ACROSS_CHUNKS)
);
