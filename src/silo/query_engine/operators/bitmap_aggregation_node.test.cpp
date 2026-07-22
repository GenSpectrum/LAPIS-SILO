#include <optional>
#include <string>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createDataWithSequences(
   const std::string& nucleotideSequence,
   const std::string& aminoAcidSequence,
   const std::string& region
) {
   random_generator generator;
   const auto primary_key = generator();
   return {
      {"primaryKey", "id_" + to_string(primary_key)},
      {"region", region},
      {"unaligned_segment1", {}},
      {"segment1", {{"sequence", nucleotideSequence}, {"insertions", nlohmann::json::array()}}},
      {"gene1", {{"sequence", aminoAcidSequence}, {"insertions", nlohmann::json::array()}}}
   };
}

// Reference is "ATGCN" / "M*". Per nucleotide position the rows carry:
//   segment1[1]: A, A, N, C
//   segment1[2]: T, T, N, A
// so the (segment1[1], segment1[2]) combinations are (A,T)x2, (N,N)x1, (C,A)x1.
// The indexed `region` column carries: Europe, Europe, Asia, Europe.
const nlohmann::json ROW_AT = createDataWithSequences("ATGCN", "M*", "Europe");
const nlohmann::json ROW_AT2 = createDataWithSequences("ATGCN", "C*", "Europe");
const nlohmann::json ROW_NN = createDataWithSequences("NNNNN", "M*", "Asia");
const nlohmann::json ROW_CA = createDataWithSequences("CATTT", "X*", "Europe");

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
    - name: "region"
      type: "string"
      generateIndex: true
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {ROW_AT, ROW_AT2, ROW_NN, ROW_CA},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

// Mutation co-occurrence is an optimizer-only feature: it is expressed with the generic `at` scalar
// function (a `map` assigns the per-position symbols and `groupBy` groups on them), and the
// BitmapAggregationRewritePass recognizes that shape and routes it through the dedicated
// bitmap-based engine. Combinations are emitted depth-first in nucleotide SYMBOLS order (A before C
// before N).
const QueryTestScenario CO_OCCURRENCE_VIA_MAP_TWO_POSITIONS = {
   .name = "CO_OCCURRENCE_VIA_MAP_TWO_POSITIONS",
   .query =
      "default.map({s1 := segment1.at(1), s2 := segment1.at(2)})"
      ".groupBy({count:=count()}, {s1, s2})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "A", "s2": "T", "count": 2},
      {"s1": "C", "s2": "A", "count": 1},
      {"s1": "N", "s2": "N", "count": 1}
   ])")
};

const QueryTestScenario CO_OCCURRENCE_VIA_MAP_WITH_FILTER = {
   .name = "CO_OCCURRENCE_VIA_MAP_WITH_FILTER",
   .query =
      "default.filter(hasMutation(position:=1, sequenceName:='segment1'))"
      ".map({s1 := segment1.at(1), s2 := segment1.at(2)})"
      ".groupBy({count:=count()}, {s1, s2})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "C", "s2": "A", "count": 1}
   ])")
};

const QueryTestScenario CO_OCCURRENCE_VIA_MAP_AMINO_ACID = {
   .name = "CO_OCCURRENCE_VIA_MAP_AMINO_ACID",
   .query = "default.map({stop := gene1.at(2)}).groupBy({count:=count()}, {stop})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"stop": "*", "count": 4}
   ])")
};

// `at` on a non-sequence column must NOT be rewritten into the bitmap engine (which only
// understands sequence columns); it has to fall back to the generic map/groupBy path. Every primary
// key starts with "id_", so the first character is 'i' for all four rows.
const QueryTestScenario CO_OCCURRENCE_VIA_MAP_NON_SEQUENCE_NOT_REWRITTEN = {
   .name = "CO_OCCURRENCE_VIA_MAP_NON_SEQUENCE_NOT_REWRITTEN",
   .query = "default.map({first := primaryKey.at(1)}).groupBy({count:=count()}, {first})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"first": "i", "count": 4}
   ])")
};

// The reference is only 5 symbols long, so position 6 is out of range. The rewritten bitmap
// aggregation node reports this when it builds the per-symbol bitmaps.
const QueryTestScenario CO_OCCURRENCE_VIA_MAP_POSITION_OUT_OF_RANGE = {
   .name = "CO_OCCURRENCE_VIA_MAP_POSITION_OUT_OF_RANGE",
   .query = "default.map({s := segment1.at(6)}).groupBy({count:=count()}, {s})",
   .expected_error_message = "SymbolInSet<Nucleotide> position is out of bounds 6 > 5"
};

// Grouping directly on an indexed string column is now routed through the bitmap engine too: the
// column is grouped straight from its inverted index. Value groups are emitted in sorted order
// (Asia before Europe). Region carries Europe x3 (the two ATGCN rows and the CATTT row) and Asia x1
// (the NNNNN row).
const QueryTestScenario INDEXED_COLUMN_SINGLE = {
   .name = "INDEXED_COLUMN_SINGLE",
   .query = "default.groupBy({count:=count()}, {region})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"region": "Asia", "count": 1},
      {"region": "Europe", "count": 3}
   ])")
};

// A sequence position and an indexed column grouped together in one node. Depth-first over the
// nucleotide symbols at segment1[1] (A, C, N), with the region value groups sorted within each.
//   A -> Europe x2   (the two ATGCN rows)
//   C -> Europe x1   (the CATTT row)
//   N -> Asia   x1   (the NNNNN row)
const QueryTestScenario MIXED_SEQUENCE_AND_INDEXED_COLUMN = {
   .name = "MIXED_SEQUENCE_AND_INDEXED_COLUMN",
   .query = "default.map({s1 := segment1.at(1)}).groupBy({count:=count()}, {s1, region})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "A", "region": "Europe", "count": 2},
      {"s1": "C", "region": "Europe", "count": 1},
      {"s1": "N", "region": "Asia", "count": 1}
   ])")
};

// A sequence-less row carries no symbol at any position. The generic `at()`/groupBy path emits a
// null group key for such a row, so the rewritten bitmap aggregation node must do the same instead
// of dropping the row or folding it into the missing symbol N/X. These scenarios pin that
// behaviour. segment1 reference is "ATGCN", gene1 reference is "M*".
//   NULL_ROW_A/B: segment1 = "ATGCN", gene1 = "M*"  (x2)
//   NULL_ROW_NO_NUC: segment1 absent, gene1 = "M*"
//   NULL_ROW_NO_AA:  segment1 = "CATTT", gene1 absent
nlohmann::json createDataWithOptionalSequences(
   const std::optional<std::string>& nucleotideSequence,
   const std::optional<std::string>& aminoAcidSequence
) {
   random_generator generator;
   const auto primary_key = generator();
   const auto sequence_field = [](const std::optional<std::string>& sequence) -> nlohmann::json {
      if (sequence.has_value()) {
         return {{"sequence", sequence.value()}, {"insertions", nlohmann::json::array()}};
      }
      return nullptr;
   };
   return {
      {"primaryKey", "id_" + to_string(primary_key)},
      {"region", "Europe"},
      {"unaligned_segment1", {}},
      {"segment1", sequence_field(nucleotideSequence)},
      {"gene1", sequence_field(aminoAcidSequence)}
   };
}

const nlohmann::json NULL_ROW_A = createDataWithOptionalSequences("ATGCN", "M*");
const nlohmann::json NULL_ROW_B = createDataWithOptionalSequences("ATGCN", "M*");
const nlohmann::json NULL_ROW_NO_NUC = createDataWithOptionalSequences(std::nullopt, "M*");
const nlohmann::json NULL_ROW_NO_AA = createDataWithOptionalSequences("CATTT", std::nullopt);

const QueryTestData NULL_TEST_DATA{
   .ndjson_input_data = {NULL_ROW_A, NULL_ROW_B, NULL_ROW_NO_NUC, NULL_ROW_NO_AA},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

// segment1[1], segment1[2]: (A,T) for the two full rows, (C,A) for the row without an amino acid
// sequence, and (null,null) for the row without a nucleotide sequence.
const QueryTestScenario CO_OCCURRENCE_NULL_TWO_NUCLEOTIDE_POSITIONS = {
   .name = "CO_OCCURRENCE_NULL_TWO_NUCLEOTIDE_POSITIONS",
   .query =
      "default.map({s1 := segment1.at(1), s2 := segment1.at(2)})"
      ".groupBy({count:=count()}, {s1, s2})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "A", "s2": "T", "count": 2},
      {"s1": "C", "s2": "A", "count": 1},
      {"s1": null, "s2": null, "count": 1}
   ])")
};

// gene1[1] is the reference symbol M for the three rows that have an amino acid sequence, and null
// for the one that does not.
const QueryTestScenario CO_OCCURRENCE_NULL_AMINO_ACID = {
   .name = "CO_OCCURRENCE_NULL_AMINO_ACID",
   .query = "default.map({aa := gene1.at(1)}).groupBy({count:=count()}, {aa})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"aa": "M", "count": 3},
      {"aa": null, "count": 1}
   ])")
};

// A combination across a nucleotide and an amino acid position: the null falls in different
// dimensions for the two partial rows, exercising a null key next to a present key.
//   full rows:        (A, M) x2
//   no amino acid:    (C, null)
//   no nucleotide:    (null, M)
const QueryTestScenario CO_OCCURRENCE_NULL_MIXED_POSITIONS = {
   .name = "CO_OCCURRENCE_NULL_MIXED_POSITIONS",
   .query =
      "default.map({s1 := segment1.at(1), aa := gene1.at(1)})"
      ".groupBy({count:=count()}, {s1, aa})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "A", "aa": "M", "count": 2},
      {"s1": "C", "aa": null, "count": 1},
      {"s1": null, "aa": "M", "count": 1}
   ])")
};

// The bitmap aggregation node emits its combinations in pipeline-sized batches
// (materialization_cutoff is the batch-size-minus-one knob). With a cutoff of 0 every combination
// is its own batch, so this exercises the multi-batch producer path and pins that batching does not
// change or reorder the result.
const QueryTestScenario CO_OCCURRENCE_NULL_CHUNKED_OUTPUT = {
   .name = "CO_OCCURRENCE_NULL_CHUNKED_OUTPUT",
   .query =
      "default.map({s1 := segment1.at(1), s2 := segment1.at(2)})"
      ".groupBy({count:=count()}, {s1, s2})",
   .expected_query_result = nlohmann::json::parse(R"([
      {"s1": "A", "s2": "T", "count": 2},
      {"s1": "C", "s2": "A", "count": 1},
      {"s1": null, "s2": null, "count": 1}
   ])"),
   .query_options = silo::config::QueryOptions{.materialization_cutoff = 0}
};

}  // namespace

QUERY_TEST(
   BitmapAggregation,
   TEST_DATA,
   ::testing::Values(
      CO_OCCURRENCE_VIA_MAP_TWO_POSITIONS,
      CO_OCCURRENCE_VIA_MAP_WITH_FILTER,
      CO_OCCURRENCE_VIA_MAP_AMINO_ACID,
      CO_OCCURRENCE_VIA_MAP_NON_SEQUENCE_NOT_REWRITTEN,
      CO_OCCURRENCE_VIA_MAP_POSITION_OUT_OF_RANGE,
      INDEXED_COLUMN_SINGLE,
      MIXED_SEQUENCE_AND_INDEXED_COLUMN
   )
);

QUERY_TEST(
   BitmapAggregationNullSequences,
   NULL_TEST_DATA,
   ::testing::Values(
      CO_OCCURRENCE_NULL_TWO_NUCLEOTIDE_POSITIONS,
      CO_OCCURRENCE_NULL_AMINO_ACID,
      CO_OCCURRENCE_NULL_MIXED_POSITIONS,
      CO_OCCURRENCE_NULL_CHUNKED_OUTPUT
   )
);
