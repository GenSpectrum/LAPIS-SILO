#pragma once

#include <filesystem>
#include <optional>

#include <fmt/format.h>

#include "config/config_metadata.h"
#include "config/ignored.h"
#include "config/toplevel_interface.h"
#include "silo/config/config_defaults.h"

namespace silo::config {

extern const ConfigStruct RUNTIME_CONFIG_METADATA;

struct ApiOptions : public OverwriteFrom {
   // XXX remove defaults, now in structs
   std::filesystem::path data_directory = silo::config::DEFAULT_OUTPUT_DIRECTORY;
   int32_t max_connections = 64;
   int32_t parallel_threads = 4;
   uint16_t port = 8081;
   std::optional<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>>
      estimated_startup_end;

   void overwriteFrom(
      const ConsList<std::string>& parents,
      const VerifiedConfigSource& config_source
   ) override;
};

struct QueryOptions : public OverwriteFrom {
   size_t materialization_cutoff = 10000;

   void overwriteFrom(
      const ConsList<std::string>& parents,
      const VerifiedConfigSource& config_source
   ) override;
};

struct RuntimeConfig : public ToplevelConfig {
   bool help;
   std::optional<Ignored> preprocessing_config;
   std::optional<std::filesystem::path> runtime_config;
   ApiOptions api_options;
   QueryOptions query_options;

   [[nodiscard]] bool asksForHelp() const override;
   [[nodiscard]] std::optional<std::filesystem::path> configPath() const override;

   void overwriteFrom(
      const ConsList<std::string>& parents,
      const VerifiedConfigSource& config_source
   ) override;

   void validate() const {};
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::RuntimeConfig> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::RuntimeConfig& runtime_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
