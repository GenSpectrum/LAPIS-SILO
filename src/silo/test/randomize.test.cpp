#include <nlohmann/json.hpp>

#include <optional>

#include "silo/test/query_fixture.test.h"

using silo::ReferenceGenomes;
using silo::config::DatabaseConfig;
using silo::config::ValueType;
using silo::test::QueryTestData;
using silo::test::QueryTestScenario;

const std::vector<nlohmann::json> DATA = {
   {{"metadata", {{"key", "id1"}, {"col", "A"}}},
    {"alignedNucleotideSequences", {{"segment1", nullptr}}},
    {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
    {"alignedAminoAcidSequences", {{"gene1", nullptr}}}},
   {{"metadata", {{"key", "id2"}, {"col", "A"}}},
    {"alignedNucleotideSequences", {{"segment1", nullptr}}},
    {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
    {"alignedAminoAcidSequences", {{"gene1", nullptr}}}},
   {{"metadata", {{"key", "id3"}, {"col", "A"}}},
    {"alignedNucleotideSequences", {{"segment1", nullptr}}},
    {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
    {"alignedAminoAcidSequences", {{"gene1", nullptr}}}},
   {{"metadata", {{"key", "id4"}, {"col", "A"}}},
    {"alignedNucleotideSequences", {{"segment1", nullptr}}},
    {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
    {"alignedAminoAcidSequences", {{"gene1", nullptr}}}},
   {{"metadata", {{"key", "id5"}, {"col", "A"}}},
    {"alignedNucleotideSequences", {{"segment1", nullptr}}},
    {"unalignedNucleotideSequences", {{"segment1", nullptr}}},
    {"alignedAminoAcidSequences", {{"gene1", nullptr}}}}
};

const auto DATABASE_CONFIG = DatabaseConfig{
   "segment1",
   {"dummy name", {{"key", ValueType::STRING}, {"col", ValueType::STRING}}, "key"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{
   {{"segment1", "A"}},
   {{"gene1", "*"}},
};

const QueryTestData TEST_DATA{DATA, DATABASE_CONFIG, REFERENCE_GENOMES};

const QueryTestScenario RANDOMIZE_SEED = {
   "seed1231ProvidedShouldShuffleResults",
   {{"action", {{"type", "Details"}, {"fields", {"key"}}, {"randomize", {{"seed", 1231}}}}},
    {"filterExpression", {{"type", "True"}}}},
   {{{"key", "id4"}}, {{"key", "id1"}}, {{"key", "id5"}}, {{"key", "id2"}}, {{"key", "id3"}}}
};

const QueryTestScenario RANDOMIZE_SEED_DIFFERENT = {
   "seed12312ProvidedShouldShuffleResultsDifferently",
   {{"action", {{"type", "Details"}, {"fields", {"key"}}, {"randomize", {{"seed", 12312}}}}},
    {"filterExpression", {{"type", "True"}}}},
   {{{"key", "id1"}}, {{"key", "id4"}}, {{"key", "id3"}}, {{"key", "id2"}}, {{"key", "id5"}}}
};

const QueryTestScenario EXPLICIT_DO_NOT_RANDOMIZE = {
   "explicitlyDoNotRandomize",
   {{"action", {{"type", "Details"}, {"fields", {"key"}}, {"randomize", false}}},
    {"filterExpression", {{"type", "True"}}}},
   {{{"key", "id1"}}, {{"key", "id2"}}, {{"key", "id3"}}, {{"key", "id4"}}, {{"key", "id5"}}}
};

const QueryTestScenario AGGREGATE = {
   "aggregateRandomize",
   {{"action",
     {{"type", "Aggregated"}, {"groupByFields", {"key"}}, {"randomize", {{"seed", 12321}}}}},
    {"filterExpression", {{"type", "True"}}}},
   {{{"count", 1}, {"key", "id3"}},
    {{"count", 1}, {"key", "id1"}},
    {{"count", 1}, {"key", "id4"}},
    {{"count", 1}, {"key", "id5"}},
    {{"count", 1}, {"key", "id2"}}}
};

QUERY_TEST(
   RandomizeTest,
   TEST_DATA,
   ::testing::Values(RANDOMIZE_SEED, RANDOMIZE_SEED_DIFFERENT, EXPLICIT_DO_NOT_RANDOMIZE, AGGREGATE)
);
