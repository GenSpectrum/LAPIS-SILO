#include <filesystem>

#include <spdlog/spdlog.h>

#include "silo/api/api.h"
#include "silo/api/logging.h"
#include "silo/append/append.h"
#include "silo/common/overloaded.h"
#include "silo/common/panic.h"
#include "silo/common/version.h"
#include "silo/config/append_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/config/runtime_config.h"
#include "silo/database.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/preprocessing/preprocessor.h"

namespace {

/// Does not throw exceptions
int runPreprocessor(const silo::config::PreprocessingConfig& preprocessing_config) {
   // TODO (#656): move body of siloPreprocessing to preprocessing.{h,cpp} #656
   try {
      auto database_config = silo::config::DatabaseConfig::getValidatedConfigFromFile(
         preprocessing_config.getDatabaseConfigFilename()
      );

      SPDLOG_INFO("preprocessing - reading reference genome");
      const auto reference_genomes =
         silo::ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

      silo::common::LineageTreeAndIdMap lineage_definitions;
      if (auto lineage_file_name = preprocessing_config.getLineageDefinitionsFilename()) {
         SPDLOG_INFO(
            "preprocessing - read and verify the lineage tree '{}'",
            lineage_file_name.value().string()
         );
         lineage_definitions = silo::common::LineageTreeAndIdMap::fromLineageDefinitionFilePath(
            lineage_file_name.value()
         );
      }

      auto preprocessor = silo::preprocessing::Initializer(
         preprocessing_config, database_config, reference_genomes, std::move(lineage_definitions)
      );
      auto database = preprocessor.initialize();

      database.saveDatabaseState(preprocessing_config.output_directory);
      return 0;
   } catch (const silo::preprocessing::PreprocessingException& preprocessing_exception) {
      SPDLOG_ERROR("Preprocessing Error: {}", preprocessing_exception.what());
      return 1;
   } catch (const std::runtime_error& error) {
      SPDLOG_ERROR("Internal Error: {}", error.what());
      return 1;
   }
}

int runApi(const silo::config::RuntimeConfig& runtime_config) {
   silo::api::Api server;
   return server.runApi(runtime_config);
}

enum class ExecutionMode { PREPROCESSING, APPEND, API };

int mainWhichMayThrowExceptions(int argc, char** argv) {
   setupLogger();

   std::vector<std::string> all_args(argv, argv + argc);

   const std::filesystem::path program_path{all_args[0]};

   const std::string program_name = program_path.filename();

   std::span<const std::string> args(all_args.begin() + 1, all_args.end());

   ExecutionMode mode;
   if (args.empty()) {
      std::cerr << program_name
                << ": need either 'preprocessing' or 'api' as the first program argument\n";
      return 1;
   }

   const std::string& mode_argument = args[0];
   args = {args.begin() + 1, args.end()};
   if (mode_argument == "preprocessing") {
      mode = ExecutionMode::PREPROCESSING;
   } else if (mode_argument == "append") {
      mode = ExecutionMode::APPEND;
   } else if (mode_argument == "api") {
      mode = ExecutionMode::API;
   } else {
      std::cerr
         << program_name
         << ": need either 'preprocessing', 'append' or 'api' as the first program argument, got '"
         << mode_argument << "'\n";
      return 1;
   }

   SPDLOG_INFO("Starting SILO (version {})", silo::RELEASE_VERSION);

   std::vector<std::string> env_allow_list;
   env_allow_list.emplace_back("SILO_PANIC");
   for (auto& field :
        silo::config::PreprocessingConfig::getConfigSpecification().attribute_specifications) {
      env_allow_list.emplace_back(
         silo::config::EnvironmentVariables::configKeyPathToString(field.key)
      );
   }
   for (auto& field :
        silo::config::RuntimeConfig::getConfigSpecification().attribute_specifications) {
      env_allow_list.emplace_back(
         silo::config::EnvironmentVariables::configKeyPathToString(field.key)
      );
   }

   switch (mode) {
      case ExecutionMode::PREPROCESSING:
         return std::visit(
            overloaded{
               [&](const silo::config::PreprocessingConfig& preprocessing_config) {
                  SPDLOG_INFO("preprocessing_config = {}", preprocessing_config);
                  return runPreprocessor(preprocessing_config);
               },
               [&](int32_t exit_code) { return exit_code; }
            },
            silo::config::getConfig<silo::config::PreprocessingConfig>(args, env_allow_list)
         );
      case ExecutionMode::APPEND:
         return std::visit(
            overloaded{
               [&](const silo::config::AppendConfig& append_config) {
                  SPDLOG_INFO("append_config = {}", append_config);
                  return runAppend(append_config);
               },
               [&](int32_t exit_code) { return exit_code; }
            },
            silo::config::getConfig<silo::config::AppendConfig>(args, env_allow_list)
         );
      case ExecutionMode::API:
         return std::visit(
            overloaded{
               [&](const silo::config::RuntimeConfig& runtime_config) {
                  SPDLOG_INFO("runtime_config = {}", runtime_config);
                  return runApi(runtime_config);
               },
               [&](int32_t exit_code) { return exit_code; }
            },
            silo::config::getConfig<silo::config::RuntimeConfig>(args, env_allow_list)
         );
   }
   SILO_UNREACHABLE();
}

}  // namespace

int main(int argc, char** argv) {
   try {
      return mainWhichMayThrowExceptions(argc, argv);
   } catch (const std::runtime_error& error) {
      SPDLOG_ERROR("Internal Error: {}", error.what());
      return 2;
   }
}
