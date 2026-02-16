#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "config/config_specification.h"
#include "config/verified_config_attributes.h"

namespace silo::config {

class PreprocessingConfig;

class InitializationFiles {
  public:
   std::vector<std::string> lineage_definition_files;
   std::optional<std::filesystem::path> phylogenetic_tree_file;
   std::filesystem::path database_config_file;
   std::filesystem::path reference_genome_file;
   bool without_unaligned_sequences;

   std::filesystem::path directory;

   [[nodiscard]] std::filesystem::path getDatabaseConfigFilepath() const;

   [[nodiscard]] std::vector<std::filesystem::path> getLineageDefinitionFilepaths() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPhyloTreeFilepath() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilepath() const;

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(
      InitializationFiles,
      directory,
      lineage_definition_files,
      phylogenetic_tree_file,
      database_config_file,
      reference_genome_file
   );
};

class InitializeConfig {
   friend class PreprocessingConfig;
   friend class fmt::formatter<silo::config::InitializeConfig>;

   InitializeConfig() = default;

  public:
   InitializationFiles initialization_files;
   std::filesystem::path output_directory;
   /// Create PreprocessingConfig with all default values from the specification
   static InitializeConfig withDefaults();

   static ConfigSpecification getConfigSpecification();

   void validate() const;

   void overwriteFrom(const VerifiedConfigAttributes& config_source);

   [[nodiscard]] static std::vector<std::filesystem::path> getConfigFilePaths(
      const VerifiedCommandLineArguments& cmd_source,
      const VerifiedConfigAttributes& env_source
   );

   NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitializeConfig, initialization_files, output_directory);
};

}  // namespace silo::config

template <>
struct [[maybe_unused]] fmt::formatter<silo::config::InitializeConfig>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::config::InitializeConfig& initialize_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
