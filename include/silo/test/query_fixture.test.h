#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/common/fmt_formatters.h"
#include "silo/common/lineage_tree.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/database_inserter.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/query_engine/query_engine.h"
#include "silo/storage/reference_genomes.h"

namespace silo::test {

/**
 * Creates a test suite for a query test.
 * The test suite executes multiple queries on the same dataset.
 *
 * @param TEST_SUITE_NAME The name of the test suite (must be unique across all generated suites).
 * @param TEST_DATA The dataset to be used for the test suite.
 * Must be an instance of silo::test::QueryTestData.
 *
 * @param TEST_VALUES The queries to be executed on the given dataset and the expected results.
 * Must be of the form `::testing::Values(silo::test::QueryTestScenario... scenarios)`.
 */
#define QUERY_TEST(TEST_SUITE_NAME, TEST_DATA, TEST_VALUES)                                        \
   namespace {                                                                                     \
   struct TEST_SUITE_NAME##DataContainer {                                                         \
      static std::shared_ptr<silo::Database> database;                                             \
      static std::unique_ptr<silo::query_engine::QueryEngine> query_engine;                        \
      static std::filesystem::path input_directory;                                                \
                                                                                                   \
      const silo::test::QueryTestData test_data = TEST_DATA;                                       \
   };                                                                                              \
   std::shared_ptr<silo::Database> TEST_SUITE_NAME##DataContainer::database;                       \
   std::unique_ptr<silo::query_engine::QueryEngine> TEST_SUITE_NAME##DataContainer::query_engine = \
      nullptr;                                                                                     \
   std::filesystem::path TEST_SUITE_NAME##DataContainer::input_directory = {};                     \
                                                                                                   \
   using TEST_SUITE_NAME##FixtureAlias =                                                           \
      silo::test::QueryTestFixture<TEST_SUITE_NAME##DataContainer>;                                \
                                                                                                   \
   INSTANTIATE_TEST_SUITE_P(                                                                       \
      TEST_SUITE_NAME,                                                                             \
      TEST_SUITE_NAME##FixtureAlias,                                                               \
      TEST_VALUES,                                                                                 \
      silo::test::printScenarioName                                                                \
   );                                                                                              \
                                                                                                   \
   TEST_P(TEST_SUITE_NAME##FixtureAlias, testQuery) {                                              \
      const auto scenario = GetParam();                                                            \
      if (!scenario.expected_error_message.empty()) {                                              \
         try {                                                                                     \
            const auto result = query_engine.executeQuery(nlohmann::to_string(scenario.query));    \
            FAIL() << "Expected an error in test case, but noting was thrown";                     \
         } catch (const std::exception& e) {                                                       \
            EXPECT_EQ(std::string(e.what()), scenario.expected_error_message);                     \
         }                                                                                         \
      } else {                                                                                     \
         const auto result = query_engine.executeQuery(nlohmann::to_string(scenario.query));       \
         const auto actual = nlohmann::json(result.entries());                                     \
         ASSERT_EQ(actual, scenario.expected_query_result);                                        \
      }                                                                                            \
   }                                                                                               \
   }  // namespace

struct QueryTestData {
   const std::vector<nlohmann::json> ndjson_input_data;
   const std::string database_config;
   const silo::ReferenceGenomes reference_genomes;
   const silo::common::LineageTreeAndIdMap lineage_tree;
};

struct QueryTestScenario {
   std::string name;
   nlohmann::json query;
   nlohmann::json expected_query_result;
   std::string expected_error_message;
};

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario);

template <typename DataContainer>
class QueryTestFixture : public ::testing::TestWithParam<QueryTestScenario> {
  public:
   static void SetUpTestSuite() {
      const DataContainer data_container;
      const QueryTestData& test_data = data_container.test_data;

      DataContainer::database = std::make_shared<Database>(
         Database{
         silo::config::DatabaseConfig::getValidatedConfig(test_data.database_config),
         test_data.lineage_tree,
         test_data.reference_genomes.nucleotide_sequence_names,
         test_data.reference_genomes.getReferenceSequences<Nucleotide>(),
         test_data.reference_genomes.aa_sequence_names,
         test_data.reference_genomes.getReferenceSequences<AminoAcid>()}
      );

      silo::DatabaseInserter database_inserter(DataContainer::database);
      silo::DatabasePartitionInserter partition_inserter = database_inserter.openNewPartition();

      for (const auto& json : test_data.ndjson_input_data) {
         partition_inserter.insert(json);
      }

      DataContainer::query_engine =
         std::make_unique<silo::query_engine::QueryEngine>(*DataContainer::database);
   }

   static void TearDownTestSuite() { std::filesystem::remove_all(DataContainer::input_directory); }

   silo::query_engine::QueryEngine& query_engine = *DataContainer::query_engine;
};

}  // namespace silo::test
