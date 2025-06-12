#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Poco/Util/OptionSet.h>
#include <fmt/format.h>

#include "config/config_interface.h"
#include "config/source/yaml_file.h"
#include "silo/common/json_type_definitions.h"
#include "silo/config/config_defaults.h"

namespace silo::config {

class AppendConfig {
   friend class fmt::formatter<silo::config::AppendConfig>;

   AppendConfig() = default;

  public:
   std::filesystem::path silo_directory;
   std::optional<std::filesystem::path> append_file;
   std::optional<std::filesystem::path> silo_data_source;

   /// Create AppendConfig with all default values from the specification
   static AppendConfig withDefaults();

   void validate() const;

   static ConfigSpecification getConfigSpecification();

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppendConfig, silo_directory, append_file, silo_data_source)
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::AppendConfig> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::AppendConfig& append_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
