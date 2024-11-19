#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Poco/Util/OptionSet.h>
#include <fmt/format.h>

#include "config/config_metadata.h"
#include "config/ignored.h"
#include "config/toplevel_interface.h"
#include "silo/config/config_defaults.h"

namespace silo::config {

extern const ConfigStruct PREPROCESSING_CONFIG_METADATA;

class PreprocessingConfig : public ToplevelConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
   bool help;
   std::optional<std::filesystem::path> preprocessing_config;
   std::optional<Ignored> runtime_config;

   std::filesystem::path input_directory;
   std::filesystem::path output_directory;
   std::filesystem::path intermediate_results_directory;
   std::optional<std::filesystem::path> preprocessing_database_location;
   std::optional<uint32_t> duckdb_memory_limit_in_g;
   std::optional<std::filesystem::path> pango_lineage_definition_file;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::optional<std::filesystem::path> metadata_file;
   std::filesystem::path database_config_file;
   std::filesystem::path reference_genome_file;
   std::string nucleotide_sequence_prefix;
   std::string unaligned_nucleotide_sequence_prefix;
   std::string gene_prefix;
   std::string nuc_insertions_filename;
   std::string aa_insertions_filename;

   void validate() const;

   [[nodiscard]] std::filesystem::path getDatabaseConfigFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPangoLineageDefinitionFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getMetadataInputFilename() const;

   [[nodiscard]] std::filesystem::path getNucFilenameNoExtension(size_t sequence_idx) const;

   [[nodiscard]] std::filesystem::path getUnalignedNucFilenameNoExtension(size_t sequence_idx
   ) const;

   [[nodiscard]] std::filesystem::path getGeneFilenameNoExtension(size_t sequence_idx) const;

   [[nodiscard]] std::filesystem::path getNucleotideInsertionsFilename() const;

   [[nodiscard]] std::filesystem::path getAminoAcidInsertionsFilename() const;

   void overwriteFromParents(
      const ConsList<std::string>& parents,
      const VerifiedConfigSource& config_source
   ) override;

   [[nodiscard]] bool asksForHelp() const override;
   [[nodiscard]] std::optional<std::filesystem::path> configPath() const override;
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
