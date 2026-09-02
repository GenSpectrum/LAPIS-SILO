#include <nlohmann/json.hpp>

#include "rhydb/test/query_fixture.test.h"

namespace {
using rhydb::ReferenceGenomes;
using rhydb::test::QueryTestData;
using rhydb::test::QueryTestScenario;

nlohmann::json createData(
   const std::string& primary_key,
   const std::string& nucleotide_sequence,
   const std::string& amino_acid_sequence
) {
   return {
      {"primaryKey", primary_key},
      {"segment1", {{"sequence", nucleotide_sequence}, {"insertions", nlohmann::json::array()}}},
      {"gene1", {{"sequence", amino_acid_sequence}, {"insertions", nlohmann::json::array()}}},
      {"gene2", nullptr}
   };
}

const auto DATABASE_CONFIG = R"(
schema:
  instanceName: "test"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

// A tiny controlled dataset for filtering the OUTPUT of the mutations() operator. The segment1
// reference is "AT"; s1 carries a single substitution A->C at position 1, s2 matches the reference.
// So `mutations(minProportion:=0.0)` produces exactly one row:
//   {mutationFrom:A, mutationTo:C, sequenceName:segment1, position:1, proportion:0.5, coverage:2,
//    count:1}
// gene1 is the reference "M*" for both rows, so it contributes no mutation rows.
const auto REFERENCE_GENOMES =
   ReferenceGenomes{{{"segment1", "AT"}}, {{"gene1", "M*"}, {"gene2", "M*"}}};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {
         createData("s1", "CT", "M*"),  // segment1 position 1: A->C
         createData("s2", "AT", "M*")   // matches the reference
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .without_unaligned_sequences = true
};

// A filter on a mutations() OUTPUT column (proportion) that keeps the single row. Regression test
// for #1372: previously a filter above mutations() was pushed into the input scan and failed with
// "database does not contain the column 'proportion'". mutations() is now a pushdown barrier, so
// the filter is retained above it and runs as an Arrow filter over its output.
const QueryTestScenario FILTER_MUTATIONS_OUTPUT_PROPORTION_KEEP = {
   .name = "FILTER_MUTATIONS_OUTPUT_PROPORTION_KEEP",
   .query = "default.mutations(minProportion:=0.0).filter(proportion > 0.4)",
   .expected_query_result = nlohmann::json::parse(R"([
      {"mutationFrom":"A","mutationTo":"C","sequenceName":"segment1","position":1,
       "proportion":0.5,"coverage":2,"count":1}
   ])")
};

// The same filter with a threshold above the row's proportion removes it, yielding an empty result
// (rather than an error) - proving the predicate really runs over the output.
const QueryTestScenario FILTER_MUTATIONS_OUTPUT_PROPORTION_DROP = {
   .name = "FILTER_MUTATIONS_OUTPUT_PROPORTION_DROP",
   .query = "default.mutations(minProportion:=0.0).filter(proportion > 0.6)",
   .expected_query_result = nlohmann::json::array()
};

// A compound filter over two output columns (count and position) keeps the row, exercising integer
// output columns and boolean combination above the mutations barrier.
const QueryTestScenario FILTER_MUTATIONS_OUTPUT_COUNT_AND_POSITION = {
   .name = "FILTER_MUTATIONS_OUTPUT_COUNT_AND_POSITION",
   .query = "default.mutations(minProportion:=0.0).filter(count = 1 && position = 1)",
   .expected_query_result = nlohmann::json::parse(R"([
      {"mutationFrom":"A","mutationTo":"C","sequenceName":"segment1","position":1,
       "proportion":0.5,"coverage":2,"count":1}
   ])")
};

}  // namespace

QUERY_TEST(
   MutationsOutputFilter,
   TEST_DATA,
   ::testing::Values(
      FILTER_MUTATIONS_OUTPUT_PROPORTION_KEEP,
      FILTER_MUTATIONS_OUTPUT_PROPORTION_DROP,
      FILTER_MUTATIONS_OUTPUT_COUNT_AND_POSITION
   )
);
