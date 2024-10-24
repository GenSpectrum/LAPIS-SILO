#include "silo/preprocessing/preprocessor.h"

#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/util/config_repository.h"
#include "silo/config/util/yaml_file.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/sql_function.h"
#include "silo/query_engine/query_engine.h"

namespace {
using namespace silo;

struct NdjsonInputLine {
   std::map<std::string, nlohmann::json> metadata;
   std::map<std::string, nlohmann::json> alignedNucleotideSequences;
   std::map<std::string, nlohmann::json> unalignedNucleotideSequences;
   std::map<std::string, nlohmann::json> alignedAminoAcidSequences;
   std::map<std::string, nlohmann::json> nucleotideInsertions;
   std::map<std::string, nlohmann::json> aminoAcidInsertions;

   [[nodiscard]] nlohmann::json toJson() const {
      return nlohmann::json{
         {"metadata", metadata},
         {"alignedNucleotideSequences", alignedNucleotideSequences},
         {"unalignedNucleotideSequences", unalignedNucleotideSequences},
         {"alignedAminoAcidSequences", alignedAminoAcidSequences},
         {"nucleotideInsertions", nucleotideInsertions},
         {"aminoAcidInsertions", aminoAcidInsertions}
      };
   }
};

struct InvalidScenario {
   std::string test_name;
   std::function<std::vector<NdjsonInputLine>()> input_data;
   silo::config::DatabaseConfig database_config;
   silo::ReferenceGenomes reference_genomes;
   silo::common::LineageTreeAndIdMap lineage_tree;
   std::string error_message;
};

std::string printTestName(const ::testing::TestParamInfo<InvalidScenario>& info) {
   std::string name = "Dir_" + info.param.test_name;
   std::ranges::replace(name, '/', '_');
   return name;
}

class InvalidPreprocessorTestFixture : public ::testing::TestWithParam<InvalidScenario> {};

const auto DATABASE_CONFIG = silo::config::DatabaseConfig{
   .schema =
      {.instance_name = "dummy name",
       .metadata = {{.name = "primaryKey", .type = silo::config::ValueType::STRING}},
       .primary_key = "primaryKey"}
};

const auto REFERENCE_GENOMES = ReferenceGenomes{};

const InvalidScenario DUPLICATE_PRIMARY_KEY{
   .test_name = "duplicate_primary_key",
   .input_data =
      []() {
         std::vector<NdjsonInputLine> result;
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_1"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_1"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_2"}}});
         result.emplace_back(NdjsonInputLine{.metadata = {{"primaryKey", "id_2"}}});
         return result;
      },
   .database_config = DATABASE_CONFIG,
   .reference_genomes = REFERENCE_GENOMES,
   .error_message = "Found 2 duplicate primary key(s): id_1, id_2"
};

INSTANTIATE_TEST_SUITE_P(
   PreprocessorTest,
   InvalidPreprocessorTestFixture,
   ::testing::Values(DUPLICATE_PRIMARY_KEY),
   printTestName
);

TEST_P(InvalidPreprocessorTestFixture, shouldNotProcessData) {
   const auto scenario = GetParam();
   auto now = std::chrono::system_clock::now();
   auto duration = now.time_since_epoch();
   auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
   const std::filesystem::path input_directory = fmt::format("test{}", millis);
   std::filesystem::create_directories(input_directory);

   const config::PreprocessingConfig config_with_input_dir{
      .input_directory = input_directory,
      .intermediate_results_directory = input_directory,
      .ndjson_input_filename = "input.json"
   };
   config_with_input_dir.validate();

   std::ofstream file(config_with_input_dir.getNdjsonInputFilename().value());

   if (!file.is_open()) {
      std::cerr << "Could not open file for writing" << std::endl;
      return;
   }
   for (const auto& line : scenario.input_data()) {
      file << line.toJson().dump() << std::endl;
   }
   file.close();

   silo::preprocessing::Preprocessor preprocessor(
      config_with_input_dir,
      scenario.database_config,
      scenario.reference_genomes,
      scenario.lineage_tree
   );
   EXPECT_THAT(
      [&]() { preprocessor.preprocess(); },
      ThrowsMessage<silo::preprocessing::PreprocessingException>(
         ::testing::HasSubstr(scenario.error_message)
      )
   );
   std::filesystem::remove_all(input_directory);
}

}  // namespace
