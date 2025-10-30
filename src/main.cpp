#include <mimalloc.h>

#include <filesystem>

#include <arrow/compute/api.h>
#include <spdlog/spdlog.h>

#include "evobench/evobench.hpp"
#include "silo/api/api.h"
#include "silo/api/logging.h"
#include "silo/append/append.h"
#include "silo/common/overloaded.h"
#include "silo/common/panic.h"
#include "silo/common/version.h"
#include "silo/config/append_config.h"
#include "silo/config/initialize_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/config/runtime_config.h"
#include "silo/database.h"
#include "silo/initialize/initialize_exception.h"
#include "silo/initialize/initializer.h"
#include "silo/preprocessing/preprocessing.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace {

/// Does not throw exceptions
int runInitializer(const silo::config::InitializeConfig& initialize_config) {
   EVOBENCH_SCOPE("top-level", "runInitializer");
   try {
      auto database =
         silo::initialize::Initializer::initializeDatabase(initialize_config.initialization_files);
      database.saveDatabaseState(initialize_config.output_directory);
      return 0;
   } catch (const silo::initialize::InitializeException& preprocessing_exception) {
      SPDLOG_ERROR("initialize - error: {}", preprocessing_exception.what());
      return 1;
   }
}

int runPreprocessor(const silo::config::PreprocessingConfig& preprocessing_config) {
   EVOBENCH_SCOPE("top-level", "runPreprocessor");
   try {
      auto database = silo::preprocessing::preprocessing(preprocessing_config);
      database.saveDatabaseState(preprocessing_config.output_directory);
      return 0;
   } catch (const silo::preprocessing::PreprocessingException& preprocessing_exception) {
      SPDLOG_ERROR("preprocessing - error: {}", preprocessing_exception.what());
      return 1;
   }
}

int runAppend(const silo::config::AppendConfig& append_config) {
   EVOBENCH_SCOPE("top-level", "runAppend");
   return silo::append::runAppend(append_config);
}

int runApi(const silo::config::RuntimeConfig& runtime_config) {
   silo::api::Api server;
   return server.runApi(runtime_config);
}

enum class ExecutionMode { INITIALIZE, APPEND, API, PREPROCESSING };

int mainWhichMayThrowExceptions(int argc, char** argv) {
   setupLogger();
   SILO_ASSERT(arrow::compute::Initialize().ok());

   std::vector<std::string> all_args(argv, argv + argc);

   const std::filesystem::path program_path{all_args[0]};

   const std::string program_name = program_path.filename();

   std::span<const std::string> args(all_args.begin() + 1, all_args.end());

   ExecutionMode mode;
   if (args.empty()) {
      std::cerr << program_name
                << ": need 'preprocessing', 'initialize', 'append' or 'api' as the first program "
                   "argument.\n";
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
   } else if (mode_argument == "initialize") {
      mode = ExecutionMode::INITIALIZE;
   } else {
      std::cerr << program_name
                << ": need 'preprocessing', 'initialize', 'append' or 'api' as the first program "
                   "argument, got '"
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
      case ExecutionMode::INITIALIZE:
         return std::visit(
            overloaded{
               [&](const silo::config::InitializeConfig& initialize_config) {
                  SPDLOG_INFO("initialize_config = {}", initialize_config);
                  return runInitializer(initialize_config);
               },
               [&](int32_t exit_code) { return exit_code; }
            },
            silo::config::getConfig<silo::config::InitializeConfig>(args, env_allow_list)
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
   // If this option is not set, memory remains very high even when no requests are sent
   // Also reduces peak memory usage under concurrency
   mi_option_set(mi_option_purge_delay, 0);

   try {
      return mainWhichMayThrowExceptions(argc, argv);
   } catch (const std::exception& error) {
      SPDLOG_ERROR("Internal Error: {}", error.what());
      return 2;
   }
}
