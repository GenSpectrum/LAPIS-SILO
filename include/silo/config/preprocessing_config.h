#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Poco/Util/OptionSet.h>
#include <fmt/format.h>

#include "silo/config/config_defaults.h"
#include "silo/config/util/abstract_config_source.h"

// Definition of the PreprocessingConfig struct and associated config
// processing code.

// TUPLE(TYPE, FIELD_NAME, DEFAULT_GENERATION, DEFAULT_VALUE, OPTION_PATH,
// PARSING_ACCESSOR_TYPE_NAME, HELP_TEXT, ACCESSOR_GENERATION, ACCESSOR_NAME)
#define PREPROCESSING_CONFIG_DEFINITION                                 \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      input_directory,                                                  \
      HAS_DEFAULT,                                                      \
      "./",                                                             \
      {"inputDirectory"},                                               \
      String,                                                           \
      "the path to the directory with the input files",                 \
      ACCESSOR,                                                         \
      getInputDirectory                                                 \
   );                                                                   \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      output_directory,                                                 \
      HAS_DEFAULT,                                                      \
      DEFAULT_OUTPUT_DIRECTORY,                                         \
      {"outputDirectory"},                                              \
      String,                                                           \
      "the path to the directory to hold the output files",             \
      ACCESSOR,                                                         \
      getOutputDirectory                                                \
   );                                                                   \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      intermediate_results_directory,                                   \
      HAS_DEFAULT,                                                      \
      "./temp/",                                                        \
      {"intermediateResultsDirectory"},                                 \
      String,                                                           \
      "the path to the directory to hold temporary files",              \
      ACCESSOR,                                                         \
      getIntermediateResultsDirectory                                   \
   );                                                                   \
   TUPLE(                                                               \
      std::optional<std::filesystem::path>,                             \
      preprocessing_database_location,                                  \
      HAS_NO_DEFAULT,                                                   \
      ,                                                                 \
      {"preprocessingDatabaseLocation"},                                \
      String,                                                           \
      "XXX?",                                                           \
      ACCESSOR,                                                         \
      getPreprocessingDatabaseLocation                                  \
   );                                                                   \
   TUPLE(                                                               \
      std::optional<uint32_t>,                                          \
      duckdb_memory_limit_in_g,                                         \
      HAS_NO_DEFAULT,                                                   \
      ,                                                                 \
      {"duckdbMemoryLimitInG"},                                         \
      UInt32,                                                           \
      "DuckDB memory limit in GB",                                      \
      ACCESSOR,                                                         \
      getDuckdbMemoryLimitInG                                           \
   );                                                                   \
   TUPLE(                                                               \
      std::optional<std::filesystem::path>,                             \
      pango_lineage_definition_file,                                    \
      HAS_NO_DEFAULT,                                                   \
      ,                                                                 \
      {"pangoLineageDefinitionFilename"},                               \
      String,                                                           \
      "file name of the file holding th pango lineage definitions",     \
      NOACCESSOR,                                                       \
      getPangoLineageDefinitionFile                                     \
   );                                                                   \
   TUPLE(                                                               \
      std::optional<std::filesystem::path>,                             \
      ndjson_input_filename,                                            \
      HAS_NO_DEFAULT,                                                   \
      ,                                                                 \
      {"ndjsonInputFilename"},                                          \
      String,                                                           \
      "file name of the file holding NDJSON input",                     \
      NOACCESSOR,                                                       \
      getNdjsonInputFilename                                            \
   );                                                                   \
   TUPLE(                                                               \
      std::optional<std::filesystem::path>,                             \
      metadata_file,                                                    \
      HAS_NO_DEFAULT,                                                   \
      ,                                                                 \
      {"metadataFilename"},                                             \
      String,                                                           \
      "file name of the file holding metadata",                         \
      NOACCESSOR,                                                       \
      getMetadataFile                                                   \
   );                                                                   \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      database_config_file,                                             \
      HAS_DEFAULT,                                                      \
      "database_config.yaml",                                           \
      {"databaseConfigFile"},                                           \
      String,                                                           \
      "file name of the file holding the database table configuration", \
      NOACCESSOR,                                                       \
      getDatabaseConfigFile                                             \
   );                                                                   \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      reference_genome_file,                                            \
      HAS_DEFAULT,                                                      \
      "reference_genomes.json",                                         \
      {"referenceGenomeFilename"},                                      \
      String,                                                           \
      "file name of the file holding the reference genome",             \
      NOACCESSOR,                                                       \
      getReferenceGenomeFile                                            \
   );                                                                   \
   TUPLE(                                                               \
      std::string,                                                      \
      nucleotide_sequence_prefix,                                       \
      HAS_DEFAULT,                                                      \
      "nuc_",                                                           \
      {"nucleotideSequencePrefix"},                                     \
      String,                                                           \
      "the prefix for nucleotide sequences",                            \
      NOACCESSOR,                                                       \
      getNucleotideSequencePrefix                                       \
   );                                                                   \
   TUPLE(                                                               \
      std::string,                                                      \
      unaligned_nucleotide_sequence_prefix,                             \
      HAS_DEFAULT,                                                      \
      "unaligned_",                                                     \
      {"unalignedNucleotideSequencePrefix"},                            \
      String,                                                           \
      "the prefix for unaligned nucleotide sequences",                  \
      NOACCESSOR,                                                       \
      getUnalignedNucleotideSequencePrefix                              \
   );                                                                   \
   TUPLE(                                                               \
      std::string,                                                      \
      gene_prefix,                                                      \
      HAS_DEFAULT,                                                      \
      "gene_",                                                          \
      {"genePrefix"},                                                   \
      String,                                                           \
      "the prefix for genes XX?",                                       \
      NOACCESSOR,                                                       \
      getGenePrefix                                                     \
   );                                                                   \
   TUPLE(                                                               \
      std::string,                                                      \
      nuc_insertions_filename,                                          \
      HAS_DEFAULT,                                                      \
      "nuc_insertions.tsv",                                             \
      {"nucleotideInsertionsFilename"},                                 \
      String,                                                           \
      "the file name of the file holding nucleotide insertions",        \
      NOACCESSOR,                                                       \
      getNucInsertionsFilename                                          \
   );                                                                   \
   TUPLE(                                                               \
      std::string,                                                      \
      aa_insertions_filename,                                           \
      HAS_DEFAULT,                                                      \
      "aa_insertions.tsv",                                              \
      {"aminoAcidInsertionsFilename"},                                  \
      String,                                                           \
      "the file name of the file hodling amino acid insertions",        \
      NOACCESSOR,                                                       \
      getAaInsertionsFilename                                           \
   )

namespace silo::config {

class PreprocessingConfig {
   friend class fmt::formatter<silo::config::PreprocessingConfig>;

  public:
#define TUPLE(                                                                          \
   TYPE,                                                                                \
   FIELD_NAME,                                                                          \
   DEFAULT_GENERATION,                                                                  \
   DEFAULT_VALUE,                                                                       \
   OPTION_PATH,                                                                         \
   PARSING_ACCESSOR_TYPE_NAME,                                                          \
   HELP_TEXT,                                                                           \
   ACCESSOR_GENERATION,                                                                 \
   ACCESSOR_NAME                                                                        \
)                                                                                       \
   ACCESSOR_GENERATION([[nodiscard]] TYPE ACCESSOR_NAME() const { return FIELD_NAME; }) \
   TYPE FIELD_NAME DEFAULT_GENERATION DEFAULT_VALUE
#define HAS_DEFAULT =
#define HAS_NO_DEFAULT
#define ACCESSOR(code) code
#define NOACCESSOR(code)

   PREPROCESSING_CONFIG_DEFINITION;

#undef NOACCESSOR
#undef ACCESSOR
#undef HAS_NO_DEFAULT
#undef HAS_DEFAULT
#undef TUPLE

   static void addOptions(Poco::Util::OptionSet& options);
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

   void overwrite(const silo::config::AbstractConfigSource& config_source);
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
