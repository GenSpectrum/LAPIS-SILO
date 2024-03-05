#include <nlohmann/json.hpp>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using boost::uuids::random_generator;

nlohmann::json createDataWithNucleotideSequence(const std::string& nucleotideSequence) {
   random_generator generator;
   const auto request_id = generator();

   return {
      {"metadata", {{"primaryKey", "id_" + to_string(request_id)}}},
      {"alignedNucleotideSequences", {{"segment1", nucleotideSequence}}},
      {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
      {"alignedAminoAcidSequences", {{"gene1", nullptr}}}
   };
}
const nlohmann::json DATA_SAME_AS_REFERENCE = createDataWithNucleotideSequence("ATGCN");
const nlohmann::json DATA_WITH_ALL_N = createDataWithNucleotideSequence("NNNNN");
const nlohmann::json DATA_WITH_ALL_MUTATED = createDataWithNucleotideSequence("CATTT");

const auto DATABASE_CONFIG =
   DatabaseConfig{"segment1", {"dummy name", {{"primaryKey", ValueType::STRING}}, "primaryKey"}};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "ATGCN"}},
   {{"gene1", "M*"}},
};

const QueryTestData TEST_DATA{
   {DATA_SAME_AS_REFERENCE, DATA_SAME_AS_REFERENCE, DATA_WITH_ALL_N, DATA_WITH_ALL_MUTATED},
   DATABASE_CONFIG,
   REFERENCE_GENOMES
};

nlohmann::json createNucleotideSymbolEqualsQuery(const std::string& symbol, int position) {
   return {
      {"action", {{"type", "Aggregated"}}},
      {"filterExpression",
       {{"type", "NucleotideEquals"},
        {"position", position},
        {"symbol", symbol},
        {"sequenceName", "segment1"}}}
   };
}

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_SYMBOL = {
   "nucleotideEqualsWithSymbol",
   createNucleotideSymbolEqualsQuery("C", 1),
   {{{"count", 1}}}
};

const QueryTestScenario NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE = {
   "nucleotideEqualsWithDot",
   createNucleotideSymbolEqualsQuery(".", 1),
   {{{"count", 2}}}
};

QUERY_TEST(
   NucleotideSymbolEquals,
   TEST_DATA,
   ::testing::Values(NUCLEOTIDE_EQUALS_WITH_SYMBOL, NUCLEOTIDE_EQUALS_WITH_DOT_RETURNS_REFERENCE)
);
