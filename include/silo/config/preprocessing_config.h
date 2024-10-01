#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <Poco/Util/OptionSet.h>
#include <fmt/format.h>

#include "silo/config/util/abstract_config_source.h"

// Definition of the PreprocessingConfig struct and associated config
// processing code.

// TUPLE(TYPE, FIELD_NAME, DEFAULT_GENERATION, DEFAULT_VALUE, OPTION_PATH,
// PARSING_ACCESSOR_TYPE_NAME, HELP_TEXT, ACCESSOR_GENERATION, ACCESSOR_NAME)
#define PREPROCESSING_CONFIG_DEFINITION                                 \
   TUPLE(                                                               \
      std::filesystem::path,                                            \
      input_directory,                                                  \
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
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
      NO,                                                               \
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
      NO,                                                               \
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
      NO,                                                               \
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
      NO,                                                               \
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
      NO,                                                               \
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
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
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
      YES,                                                              \
      "aa_insertions.tsv",                                              \
      {"aminoAcidInsertionsFilename"},                                  \
      String,                                                           \
      "the file name of the file hodling amino acid insertions",        \
      NOACCESSOR,                                                       \
      getAaInsertionsFilename                                           \
   )

namespace silo::config {

static const std::string DEFAULT_OUTPUT_DIRECTORY = "./output/";

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
#define YES =
#define NO
#define ACCESSOR(code) code
#define NOACCESSOR(code)

   PREPROCESSING_CONFIG_DEFINITION;

#undef NOACCESSOR
#undef ACCESSOR
#undef NO
#undef YES
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
