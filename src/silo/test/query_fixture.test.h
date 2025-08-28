#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <gtest/gtest.h>
#include <simdjson.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/append/database_inserter.h"
#include "silo/common/fmt_formatters.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/phylo_tree.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/initialize/initializer.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_plan.h"
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
#define QUERY_TEST(TEST_SUITE_NAME, TEST_DATA, TEST_VALUES)                                      \
   struct TEST_SUITE_NAME##DataContainer {                                                       \
      const silo::test::QueryTestData test_data = TEST_DATA;                                     \
   };                                                                                            \
                                                                                                 \
   using TEST_SUITE_NAME##FixtureAlias =                                                         \
      silo::test::QueryTestFixture<TEST_SUITE_NAME##DataContainer>;                              \
                                                                                                 \
   template <>                                                                                   \
   std::shared_ptr<silo::Database> TEST_SUITE_NAME##FixtureAlias::shared_database = nullptr;     \
                                                                                                 \
   INSTANTIATE_TEST_SUITE_P(                                                                     \
      TEST_SUITE_NAME, TEST_SUITE_NAME##FixtureAlias, TEST_VALUES, silo::test::printScenarioName \
   );                                                                                            \
                                                                                                 \
   TEST_P(TEST_SUITE_NAME##FixtureAlias, testQuery) {                                            \
      const auto scenario = GetParam();                                                          \
      runTest(scenario);                                                                         \
   };

struct QueryTestData {
   const std::vector<nlohmann::json> ndjson_input_data;
   const std::string database_config;
   const silo::ReferenceGenomes reference_genomes;
   const silo::common::LineageTreeAndIdMap lineage_tree;
   const silo::common::PhyloTree phylo_tree_file;
};

struct QueryTestScenario {
   std::string name;
   nlohmann::json query;
   nlohmann::json expected_query_result;
   std::string expected_error_message;
   std::optional<silo::config::QueryOptions> query_options;
};

std::string printScenarioName(const ::testing::TestParamInfo<QueryTestScenario>& scenario);

template <typename DataContainer>
class QueryTestFixture : public ::testing::TestWithParam<QueryTestScenario> {
  public:
   static std::shared_ptr<silo::Database> shared_database;

   static void SetUpTestSuite() {
      const DataContainer data_container;
      const QueryTestData& test_data = data_container.test_data;

      auto database = std::make_shared<Database>(
         Database{silo::initialize::Initializer::createSchemaFromConfigFiles(
            silo::config::DatabaseConfig::getValidatedConfig(test_data.database_config),
            std::move(test_data.reference_genomes),
            std::move(test_data.lineage_tree),
            std::move(test_data.phylo_tree_file)
         )}
      );

      std::stringstream ndjson_objects;
      for (const auto& object : test_data.ndjson_input_data) {
         ndjson_objects << object.dump() << "\n";
      }
      std::istream ndjson_objects_istream(ndjson_objects.rdbuf());

      silo::append::NdjsonLineReader reader{ndjson_objects_istream};
      silo::append::appendDataToDatabase(*database, reader);

      shared_database = database;
   }

   void runTest(silo::test::QueryTestScenario scenario) {
      if (!shared_database) {
         FAIL() << "There was an error when setting up the test suite. Database not initialized.";
      }
      const auto query_options =
         scenario.query_options.value_or(config::RuntimeConfig::withDefaults().query_options);
      if (!scenario.expected_error_message.empty()) {
         try {
            auto query = query_engine::Query::parseQuery(scenario.query.dump());
            std::stringstream buffer;
            auto query_plan = query->toQueryPlan(shared_database, query_options);
            query_plan.executeAndWrite(&buffer, /*timeout_in_seconds=*/3);
            FAIL() << "Expected an error in test case, but nothing was thrown";
         } catch (const std::exception& e) {
            EXPECT_EQ(std::string(e.what()), scenario.expected_error_message);
         }
      } else {
         auto query = query_engine::Query::parseQuery(scenario.query.dump());
         std::stringstream buffer;
         auto query_plan = query->toQueryPlan(shared_database, query_options);
         query_plan.executeAndWrite(&buffer, /*timeout_in_seconds=*/3);
         nlohmann::json actual_ndjson_result_as_array = nlohmann::json::array();
         std::string line;
         while (std::getline(buffer, line)) {
            auto line_object = nlohmann::json::parse(line);
            std::cout << line_object.dump() << std::endl;
            actual_ndjson_result_as_array.push_back(line_object);
         }
         ASSERT_EQ(actual_ndjson_result_as_array, scenario.expected_query_result);
      }
   }
};

nlohmann::json negateFilter(const nlohmann::json& filter);

}  // namespace silo::test
