#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <fmt/format.h>

#include "silo/config/initialize_config.h"

namespace silo::config {

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
   InitializationFiles initialization_files;
   std::optional<std::filesystem::path> input_file;
   std::filesystem::path output_directory;

   /// Create PreprocessingConfig with all default values from the specification
   static PreprocessingConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   [[nodiscard]] std::optional<std::filesystem::path> getInputFilePath() const;

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(PreprocessingConfig, initialization_files, input_file)
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::PreprocessingConfig>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::PreprocessingConfig& preprocessing_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
