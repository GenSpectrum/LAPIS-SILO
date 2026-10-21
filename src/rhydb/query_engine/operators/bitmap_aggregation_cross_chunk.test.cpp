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

// A template row: a unique primary key and a null `segment1`. Template-only
// (padding) rows keep the null sequence, so they all group as `s1 = null`.
nlohmann::json createTemplate(size_t chunk_index, size_t row_index) {
   return {
      {"primaryKey", "id_c" + std::to_string(chunk_index) + "_" + std::to_string(row_index)},
      {"segment1", nullptr},
   };
}

// A `segment1` column value carrying `nucleotide_sequence`, overlaid on the
// template for real rows. The symbol at position 1 decides the row's group.
nlohmann::json segment1Value(const std::string& nucleotide_sequence) {
   return {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}};
}

// The real rows per chunk, as column overrides laid over the template. The same
// first-position symbol appears in several chunks (A in all three; C in chunks
// 0 and 1; T in chunks 1 and 2), so the asserted totals can only be produced by
// folding the per-chunk counts together.
const std::vector<std::vector<nlohmann::json>> VALUES = {
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
};

// Counts the real rows whose segment1 sequence starts with `symbol`, across all
// chunks. Keeps the expected result in sync with VALUES.
int totalForSymbol(char symbol) {
   int total = 0;
   for (const auto& chunk_values : VALUES) {
      for (const auto& row : chunk_values) {
         const auto& sequence = row.at("segment1").at("sequence").get_ref<const std::string&>();
         if (!sequence.empty() && sequence[0] == symbol) {
            ++total;
         }
      }
   }
   return total;
}

const rhydb::test::LargeDataset DATASET = buildAcrossChunks(createTemplate, VALUES);

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
      {{"s1", "T"}, {"count", totalForSymbol('T')}},
      {{"s1", nullptr}, {"count", static_cast<int64_t>(DATASET.filler_count)}},
   }),
};
}  // namespace

QUERY_TEST(
   BitmapAggregationCrossChunk,
   TEST_DATA,
   ::testing::Values(GROUP_BY_POSITION_ACROSS_CHUNKS)
);
