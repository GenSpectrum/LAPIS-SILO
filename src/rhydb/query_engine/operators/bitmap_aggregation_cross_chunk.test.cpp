#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "rhydb/storage/reference_genomes.h"
#include "rhydb/test/large_dataset.test.h"
#include "rhydb/test/query_fixture.test.h"

// Bitmap aggregation computes group counts one chunk at a time and folds the
// per-chunk results together. All other bitmap-aggregation tests use a handful
// of rows, so they only ever touch a single chunk and never exercise the
// cross-chunk fold. These datasets deliberately span three ingestion chunks:
// the same groups appear in several chunks, so the asserted counts only match
// if the fold across chunks is correct. The final chunk is left partial, which
// also covers the non-full-chunk path.

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::buildAcrossChunks;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

// A string primary key and a single nucleotide segment `segment1`.
const auto DATABASE_CONFIG =
   R"(
schema:
  instanceName: "large dataset"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

// The reference is intentionally short to keep the large datasets cheap.
const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {},
};

// A real row carrying `nucleotide_sequence` on `segment1`.
nlohmann::json createRow(const std::string& primary_key, const std::string& nucleotide_sequence) {
   return {
      {"primaryKey", primary_key},
      {"segment1", {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}}},
   };
}

// A filler row with a null `segment1`. All filler rows group as `s1 = null`.
nlohmann::json createFillerRow(size_t index) {
   return {
      {"primaryKey", "filler_" + std::to_string(index)},
      {"segment1", nullptr},
   };
}

// The reference is "ATGCN". We group by segment1.at(1), so the symbol at the
// first position of each row's sequence decides its group. Build sequences that
// put a chosen symbol at position 1 and keep the rest at the reference.
std::string sequenceWithFirstSymbol(char symbol) {
   std::string sequence = "ATGCN";
   sequence[0] = symbol;
   return sequence;
}

// Per group, how many real rows go into each of the three chunks. The same
// group appears across chunks, so the totals can only be produced by folding
// per-chunk counts together.
struct GroupLayout {
   char symbol;
   std::array<int, 3> rows_per_chunk;
};

constexpr std::array<GroupLayout, 3> GROUP_LAYOUTS{{
   {'A', {3, 5, 2}},  // total 10
   {'C', {1, 0, 4}},  // total 5, absent from the middle chunk
   {'G', {0, 2, 6}},  // total 8, absent from the first chunk
}};

int totalForSymbol(char symbol) {
   for (const auto& layout : GROUP_LAYOUTS) {
      if (layout.symbol == symbol) {
         return layout.rows_per_chunk[0] + layout.rows_per_chunk[1] + layout.rows_per_chunk[2];
      }
   }
   return 0;
}
rhydb::test::LargeDataset buildDataset() {
   std::vector<std::vector<nlohmann::json>> chunks(3);
   size_t primary_key_counter = 0;
   for (size_t chunk_index = 0; chunk_index < chunks.size(); ++chunk_index) {
      for (const auto& layout : GROUP_LAYOUTS) {
         for (int i = 0; i < layout.rows_per_chunk[chunk_index]; ++i) {
            chunks[chunk_index].push_back(createRow(
               "id_" + std::to_string(primary_key_counter++), sequenceWithFirstSymbol(layout.symbol)
            ));
         }
      }
   }
   return buildAcrossChunks(chunks, createFillerRow);
}

const rhydb::test::LargeDataset DATASET = buildDataset();

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
      {{"s1", "A"}, {"count", totalForSymbol('A')}},
      {{"s1", "C"}, {"count", totalForSymbol('C')}},
      {{"s1", "G"}, {"count", totalForSymbol('G')}},
      {{"s1", nullptr}, {"count", static_cast<int64_t>(DATASET.filler_count)}},
   }),
};
}  // namespace

QUERY_TEST(
   BitmapAggregationCrossChunk,
   TEST_DATA,
   ::testing::Values(GROUP_BY_POSITION_ACROSS_CHUNKS)
);
