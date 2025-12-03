#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

using boost::uuids::random_generator;

nlohmann::json createDataWithNucleotideSequence(const std::string& nucleotideSequence) {
   random_generator generator;
   const auto primary_key = generator();

   return {
      {"primaryKey", "id_" + to_string(primary_key)},
      {"float_value", nullptr},
      {"segment1", {{"sequence", nucleotideSequence}, {"insertions", nlohmann::json::array()}}},
      {"unaligned_segment1", {}},
      {"gene1", {}}
   };
}

const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_WITH_ALL_N = createDataWithNucleotideSequence("NNNNN");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithNucleotideSequence("CATTT");

const auto DATABASE_CONFIG =
   R"(
defaultNucleotideSequence: "segment1"
schema:
  instanceName: "dummy name"
  metadata:
    - name: "primaryKey"
      type: "string"
  primaryKey: "primaryKey"
)";

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data =
      {DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_ALL_N, DATA_WITH_ALL_MUTATED},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

nlohmann::json createNucleotideSymbolEqualsQuery(const std::string& symbol, int position) {
   return nlohmann::json::parse(fmt::format(
      R"(
{{
  "action": {{
    "type": "Aggregated"
  }},
  "filterExpression": {{
    "type": "NucleotideEquals",
    "position": {},
    "symbol": "{}",
    "sequenceName": "segment1"
  }}
}}
)",
      position,
      symbol
   ));
}

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL",
   .query = createNucleotideSymbolEqualsQuery("C", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE = {
   .name = "NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE",
   .query = createNucleotideSymbolEqualsQuery(".", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_2 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_2",
   .query = createNucleotideSymbolEqualsQuery("C", 2),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_3 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_3",
   .query = createNucleotideSymbolEqualsQuery("C", 3),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_4 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_4",
   .query = createNucleotideSymbolEqualsQuery("C", 4),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_5 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_5",
   .query = createNucleotideSymbolEqualsQuery("C", 5),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_1 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_1",
   .query = createNucleotideSymbolEqualsQuery("A", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_2 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_2",
   .query = createNucleotideSymbolEqualsQuery("A", 2),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_3 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_3",
   .query = createNucleotideSymbolEqualsQuery("A", 3),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_4 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_4",
   .query = createNucleotideSymbolEqualsQuery("A", 4),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_5 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_5",
   .query = createNucleotideSymbolEqualsQuery("A", 5),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_1 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_1",
   .query = createNucleotideSymbolEqualsQuery("G", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_2 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_2",
   .query = createNucleotideSymbolEqualsQuery("G", 2),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_3 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_3",
   .query = createNucleotideSymbolEqualsQuery("G", 3),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_4 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_4",
   .query = createNucleotideSymbolEqualsQuery("G", 4),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_5 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_5",
   .query = createNucleotideSymbolEqualsQuery("G", 5),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_1 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_1",
   .query = createNucleotideSymbolEqualsQuery("T", 1),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 0}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_2 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_2",
   .query = createNucleotideSymbolEqualsQuery("T", 2),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_3 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_3",
   .query = createNucleotideSymbolEqualsQuery("T", 3),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_4 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_4",
   .query = createNucleotideSymbolEqualsQuery("T", 4),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_5 = {
   .name = "NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_5",
   .query = createNucleotideSymbolEqualsQuery("T", 5),
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario NUCLEOTIDE_EQUALS_SYMBOL_OUT_OF_RANGE = {
   .name = "NUCLEOTIDE_EQUALS_SYMBOL_OUT_OF_RANGE",
   .query = createNucleotideSymbolEqualsQuery("C", 1000),
   .expected_error_message = "SymbolEquals<Nucleotide> position is out of bounds 1000 > 5"
};

const QueryTestScenario NUCLEOTIDE_EQUALS_OUT_OF_RANGE_EDGE_LOW = {
   .name = "NUCLEOTIDE_EQUALS_OUT_OF_RANGE_EDGE_LOW",
   .query = createNucleotideSymbolEqualsQuery(".", 0),
   .expected_error_message = "The field 'position' is 1-indexed. Value of 0 not allowed."
};

}  // namespace

QUERY_TEST(
   NucleotideSymbolEquals,
   TEST_DATA,
   ::testing::Values(
      NUCLEOTIDE_EQUALS_WITH_SYMBOL,
      NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE,
      NUCLEOTIDE_EQUALS_SYMBOL_OUT_OF_RANGE,
      NUCLEOTIDE_EQUALS_OUT_OF_RANGE_EDGE_LOW,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_2,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_3,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_4,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_5,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_1,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_2,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_3,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_4,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_5,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_1,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_2,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_3,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_4,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_5,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_1,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_2,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_3,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_4,
      NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_5
   )
);
