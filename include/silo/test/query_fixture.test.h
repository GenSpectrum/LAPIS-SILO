#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_config_reader.h"
#include "silo/preprocessing/preprocessor.h"
#include "silo/preprocessing/sql_function.h"
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
      static std::unique_ptr<silo::Database> database;                                             \
      static std::unique_ptr<silo::query_engine::QueryEngine> query_engine;                        \
      static std::filesystem::path input_directory;                                                \
                                                                                                   \
      const silo::test::QueryTestData test_data = TEST_DATA;                                       \
   };                                                                                              \
   std::unique_ptr<silo::Database> TEST_SUITE_NAME##DataContainer::database = nullptr;             \
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
      const auto result = query_engine.executeQuery(nlohmann::to_string(scenario.query));          \
      const auto actual = nlohmann::json(result.query_result);                                     \
      ASSERT_EQ(actual, scenario.expected_query_result);                                           \
   }                                                                                               \
   }  // namespace

struct QueryTestData {
   const std::vector<nlohmann::json> ndjson_input_data;
   const silo::config::DatabaseConfig database_config;
   const silo::ReferenceGenomes reference_genomes;
};

struct QueryTestScenario {
   std::string name;
   nlohmann::json query;
   nlohmann::json expected_query_result;
};

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario);

template <typename DataContainer>
class QueryTestFixture : public ::testing::TestWithParam<QueryTestScenario> {
  public:
   static void SetUpTestSuite() {
      auto now = std::chrono::system_clock::now();
      auto duration = now.time_since_epoch();
      auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

      auto config_with_input_dir = silo::preprocessing::OptionalPreprocessingConfig{};
      config_with_input_dir.input_directory = {"test" + std::to_string(millis)};
      config_with_input_dir.ndjson_input_filename = "input.ndjson";
      config_with_input_dir.intermediate_results_directory =
         config_with_input_dir.input_directory.value() / "temp";

      DataContainer::input_directory = config_with_input_dir.input_directory.value();

      const DataContainer data_container;
      const QueryTestData& test_data = data_container.test_data;

      std::filesystem::create_directories(config_with_input_dir.input_directory.value());

      auto preprocessing_config =
         silo::preprocessing::OptionalPreprocessingConfig{}.mergeValuesFromOrDefault(
            config_with_input_dir
         );

      std::ofstream file(preprocessing_config.getNdjsonInputFilename().value());

      if (!file.is_open()) {
         std::cerr << "Could not open file for writing" << std::endl;
         return;
      }
      for (const auto json : test_data.ndjson_input_data) {
         file << json.dump() << std::endl;
      }
      file.close();

      silo::preprocessing::Preprocessor preprocessor(
         preprocessing_config, test_data.database_config, test_data.reference_genomes
      );
      DataContainer::database = std::make_unique<Database>(preprocessor.preprocess());

      DataContainer::query_engine =
         std::make_unique<silo::query_engine::QueryEngine>(*DataContainer::database);
   }

   static void TearDownTestSuite() { std::filesystem::remove_all(DataContainer::input_directory); }

   silo::query_engine::QueryEngine& query_engine = *DataContainer::query_engine;
};

}  // namespace silo::test
