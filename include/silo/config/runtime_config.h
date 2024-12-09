#pragma once

#include <filesystem>
#include <optional>

#include <fmt/format.h>

#include "config/config_interface.h"
#include "config/config_source_interface.h"
#include "config/config_specification.h"
#include "silo/config/config_defaults.h"

namespace silo::config {

class ApiOptions {
  public:
   int32_t max_connections;
   int32_t parallel_threads;
   uint16_t port;
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
      estimated_startup_end;
};

class QueryOptions {
  public:
   size_t materialization_cutoff;
};

class RuntimeConfig {
   RuntimeConfig() = default;

  public:
   std::filesystem::path data_directory;
   ApiOptions api_options;
   QueryOptions query_options;

   static RuntimeConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const {};

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   void overwriteFrom(const VerifiedConfigAttributes& config_source);
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::RuntimeConfig> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::RuntimeConfig& runtime_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
