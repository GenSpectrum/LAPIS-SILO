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

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

   PreprocessingConfig() = default;

   std::optional<uint32_t> duckdb_memory_limit_in_g;
   std::optional<std::filesystem::path> lineage_definitions_file;
   std::filesystem::path database_config_file;
   std::filesystem::path reference_genome_file;

  public:
   std::filesystem::path input_directory;
   std::filesystem::path output_directory;
   std::filesystem::path intermediate_results_directory;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::optional<std::filesystem::path> preprocessing_database_location;

   /// Create PreprocessingConfig with all default values from the specification
   static PreprocessingConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   [[nodiscard]] std::filesystem::path getDatabaseConfigFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getLineageDefinitionsFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   [[nodiscard]] std::optional<uint32_t> getDuckdbMemoryLimitInG() const;

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(
      PreprocessingConfig,
      input_directory,
      output_directory,
      intermediate_results_directory,
      preprocessing_database_location,
      duckdb_memory_limit_in_g,
      lineage_definitions_file,
      ndjson_input_filename,
      database_config_file,
      reference_genome_file
   )
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
