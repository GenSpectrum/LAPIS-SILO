#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

namespace {
using silo::ReferenceGenomes;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

namespace nucleotide {

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

std::string createNucleotideSymbolEqualsQuery(const std::string& symbol, int position) {
   return fmt::format(
      "default.filter(nucleotideEquals(position:={}, symbol:='{}', "
      "sequenceName:='segment1')).groupBy({{count:=count()}})",
      position,
      symbol
   );
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

}  // namespace nucleotide

namespace amino_acid {

const std::string GENE = "gene1";

size_t idx = 0;

nlohmann::json createDataWithAminoAcidSequence(const std::string& aminoAcidSequence) {
   return {
      {"primaryKey", fmt::format("id_{}", idx++)},
      {"segment1", nullptr},
      {GENE, {{"sequence", aminoAcidSequence}, {"insertions", nlohmann::json::array()}}},
      {"unaligned_segment1", {}}
   };
}
const nlohmann::json DATA_WITH_D = createDataWithAminoAcidSequence("D*");
const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_SAME_AS_REFERENCE2 = createDataWithAminoAcidSequence("M*");
const nlohmann::json DATA_WITH_B = createDataWithAminoAcidSequence("B*");

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
   {{"segment1", "A"}},
   {{GENE, "M*"}},
};

const QueryTestData TEST_DATA{
   .ndjson_input_data = {DATA_WITH_D, DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE2, DATA_WITH_B},
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES
};

const QueryTestScenario AMINO_ACID_EQUALS_D = {
   .name = "AMINO_ACID_EQUALS_D",
   .query =
      "default.filter(aminoAcidEquals(position:=1, symbol:='D', sequenceName:='gene1'))"
      ".groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count": 1}])")
};

const QueryTestScenario AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE = {
   .name = "AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE",
   .query =
      "default.filter(aminoAcidEquals(position:=1, symbol:='.', sequenceName:='gene1'))"
      ".groupBy({count:=count()})",
   .expected_query_result = nlohmann::json::parse(R"([{"count": 2}])")
};

}  // namespace amino_acid

}  // namespace

QUERY_TEST(
   NucleotideSymbolEquals,
   nucleotide::TEST_DATA,
   ::testing::Values(
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE,
      nucleotide::NUCLEOTIDE_EQUALS_SYMBOL_OUT_OF_RANGE,
      nucleotide::NUCLEOTIDE_EQUALS_OUT_OF_RANGE_EDGE_LOW,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_2,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_3,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_4,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_C_AT_5,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_1,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_2,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_3,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_4,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_A_AT_5,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_1,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_2,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_3,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_4,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_G_AT_5,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_1,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_2,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_3,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_4,
      nucleotide::NUCLEOTIDE_EQUALS_WITH_SYMBOL_T_AT_5
   )
);

QUERY_TEST(
   AminoAcidSymbolEquals,
   amino_acid::TEST_DATA,
   ::testing::Values(
      amino_acid::AMINO_ACID_EQUALS_D,
      amino_acid::AMINO_ACID_EQUALS_WITH_DOT_RETURNS_AS_IF_REFERENCE
   )
);
