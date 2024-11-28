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
   std::optional<bool> help;
   std::optional<std::filesystem::path> default_runtime_config;
   std::optional<std::filesystem::path> runtime_config;

  public:
   std::filesystem::path data_directory;
   ApiOptions api_options;
   QueryOptions query_options;

   RuntimeConfig();

   static ConfigSpecification getConfigSpecification();

   void validate() const {};

   [[nodiscard]] bool asksForHelp() const;

   [[nodiscard]] std::vector<std::filesystem::path> getConfigPaths() const;

   void overwriteFrom(const VerifiedConfigSource& config_source);
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::ApiOptions> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::ApiOptions& api_options,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::QueryOptions> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::QueryOptions& query_options,
      format_context& ctx
   ) -> decltype(ctx.out());
};

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::RuntimeConfig> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::RuntimeConfig& runtime_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
