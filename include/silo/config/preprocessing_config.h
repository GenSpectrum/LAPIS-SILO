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

namespace silo::config {

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

   PreprocessingConfig() = default;

   std::optional<std::filesystem::path> lineage_definitions_file;
   std::filesystem::path database_config_file;
   std::filesystem::path reference_genome_file;
   std::filesystem::path input_directory;

  public:
   std::filesystem::path output_directory;
   /// Create PreprocessingConfig with all default values from the specification
   static PreprocessingConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   [[nodiscard]] std::filesystem::path getDatabaseConfigFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getLineageDefinitionsFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(
      PreprocessingConfig,
      input_directory,
      lineage_definitions_file,
      database_config_file,
      reference_genome_file,
      output_directory
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
