#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/format.h>

#include "config/config_interface.h"
#include "config/source/yaml_file.h"
#include "silo/common/json_type_definitions.h"
#include "silo/config/config_defaults.h"
#include "silo/config/initialize_config.h"

namespace silo::config {

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
   InitializeConfig initialize_config;
   std::optional<std::filesystem::path> input_file;

   /// Create PreprocessingConfig with all default values from the specification
   static PreprocessingConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(PreprocessingConfig, initialize_config, input_file)
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
