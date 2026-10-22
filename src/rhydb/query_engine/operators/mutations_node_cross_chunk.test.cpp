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

nlohmann::json createRow(size_t row_index) {
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

const QueryTestScenario MUTATIONS_ACROSS_CHUNKS = {
   .name = "MUTATIONS_ACROSS_CHUNKS",
   .query = "data.mutations(minProportion:=0.0).order(by:={position, mutationTo})",
   .expected_query_result = nlohmann::json::array({
      {
         {"mutationFrom", "A"},
         {"mutationTo", "C"},
         {"sequenceName", "segment1"},
         {"position", 1},
         {"proportion", 0.5},
         {"coverage", 400'000},
         {"count", 200'000},
      },
      {
         {"mutationFrom", "A"},
         {"mutationTo", "G"},
         {"sequenceName", "segment1"},
         {"position", 1},
         {"proportion", 0.25},
         {"coverage", 400'000},
         {"count", 100'000},
      },
   }),
};
}  // namespace

QUERY_TEST(MutationsNodeCrossChunk, TEST_DATA, ::testing::Values(MUTATIONS_ACROSS_CHUNKS));
