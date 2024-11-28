#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Poco/Util/OptionSet.h>
#include <fmt/format.h>

#include "config/source/yaml_file.h"
#include "config/config_interface.h"
#include "silo/config/config_defaults.h"

namespace silo::config {

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

   std::optional<bool> help;
   std::optional<std::filesystem::path> preprocessing_config;

  public:
   std::filesystem::path input_directory;
   std::filesystem::path output_directory;
   std::filesystem::path intermediate_results_directory;
   std::optional<std::filesystem::path> preprocessing_database_location;
   std::optional<uint32_t> duckdb_memory_limit_in_g;
   std::optional<std::filesystem::path> lineage_definitions_file;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::filesystem::path database_config_file;
   std::filesystem::path reference_genome_file;

   /// Create PreprocessingConfig with all default values from the specification
   PreprocessingConfig();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   [[nodiscard]] std::filesystem::path getDatabaseConfigFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getLineageDefinitionsFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   [[nodiscard]] std::optional<uint32_t> getDuckdbMemoryLimitInG() const;

   [[nodiscard]] bool asksForHelp() const;

   void overwriteFrom(const silo::config::VerifiedConfigSource& config_source);

   [[nodiscard]] std::optional<std::filesystem::path> configPath() const;
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
