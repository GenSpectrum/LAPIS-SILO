#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <fmt/core.h>

namespace silo::preprocessing {

constexpr std::string_view ZSTDFASTA_EXTENSION(".zstdfasta");
constexpr std::string_view FASTA_EXTENSION(".fasta");
constexpr std::string_view TSV_EXTENSION(".tsv");

struct PartitionChunk;
struct Partitions;

struct InputDirectory {
   std::string directory;
};
const InputDirectory DEFAULT_INPUT_DIRECTORY = {"./"};

struct OutputDirectory {
   std::string directory;
};
const OutputDirectory DEFAULT_OUTPUT_DIRECTORY = {"./output/"};

struct IntermediateResultsDirectory {
   std::string directory;
};
const OutputDirectory DEFAULT_INTERMEDIATE_RESULTS_DIRECTORY = {"./temp/"};

struct MetadataFilename {
   std::string filename;
};
const MetadataFilename DEFAULT_METADATA_FILENAME = {"metadata.tsv"};

struct NdjsonInputFilename {
   std::optional<std::string> filename;
};

struct PreprocessingDatabaseLocation {
   std::optional<std::string> filename;
};

struct PangoLineageDefinitionFilename {
   std::optional<std::string> filename;
};

struct NucleotideSequencePrefix {
   std::string prefix;
};
const NucleotideSequencePrefix DEFAULT_NUCLEOTIDE_SEQUENCE_PREFIX = {"nuc_"};

struct UnalignedNucleotideSequencePrefix {
   std::string prefix;
};
const UnalignedNucleotideSequencePrefix DEFAULT_UNALIGNED_NUC_SEQUENCE_PREFIX = {"unaligned_"};

struct GenePrefix {
   std::string prefix;
};
const GenePrefix DEFAULT_GENE_PREFIX = {"gene_"};

struct ReferenceGenomeFilename {
   std::string filename;
};
const ReferenceGenomeFilename DEFAULT_REFERENCE_GENOME_FILENAME = {"reference_genomes.json"};

class PreprocessingConfig {
   friend class fmt::formatter<silo::preprocessing::PreprocessingConfig>;

   std::filesystem::path input_directory;
   std::filesystem::path intermediate_results_directory;
   std::filesystem::path output_directory;
   std::optional<std::filesystem::path> preprocessing_database_location;
   std::optional<std::filesystem::path> pango_lineage_definition_file;
   std::optional<std::filesystem::path> ndjson_input_filename;
   std::filesystem::path metadata_file;
   std::filesystem::path sequences_folder;
   std::filesystem::path reference_genome_file;
   std::string nucleotide_sequence_prefix;
   std::string unaligned_nucleotide_sequence_prefix;
   std::string gene_prefix;

  public:
   explicit PreprocessingConfig();

   explicit PreprocessingConfig(
      const InputDirectory& input_directory_,
      const IntermediateResultsDirectory& intermediate_results_directory_,
      const OutputDirectory& output_directory_,
      const PreprocessingDatabaseLocation& preprocessing_database_location_,
      const NdjsonInputFilename& ndjson_input_filename_,
      const MetadataFilename& metadata_filename_,
      const PangoLineageDefinitionFilename& pango_lineage_definition_filename_,
      const ReferenceGenomeFilename& reference_genome_filename_,
      const NucleotideSequencePrefix& nucleotide_sequence_prefix_,
      const UnalignedNucleotideSequencePrefix& unaligned_nucleotide_sequence_prefix_,
      const GenePrefix& gene_prefix_
   );

   [[nodiscard]] std::filesystem::path getOutputDirectory() const;

   [[nodiscard]] std::filesystem::path getIntermediateResultsDirectory() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPangoLineageDefinitionFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::optional<std::filesystem::path> getPreprocessingDatabaseLocation() const;

   [[nodiscard]] std::optional<std::filesystem::path> getNdjsonInputFilename() const;

   [[nodiscard]] std::filesystem::path getMetadataInputFilename() const;

   [[nodiscard]] std::filesystem::path getNucFilenameNoExtension(std::string_view nuc_name) const;

   [[nodiscard]] std::filesystem::path getUnalignedNucFilenameNoExtension(std::string_view nuc_name
   ) const;

   [[nodiscard]] std::filesystem::path getGeneFilenameNoExtension(std::string_view gene_name) const;
};

std::filesystem::path createPath(
   const std::filesystem::path& directory,
   const std::string& filename
);

}  // namespace silo::preprocessing

template <>
struct [[maybe_unused]] fmt::formatter<silo::preprocessing::PreprocessingConfig>
    : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(
      const silo::preprocessing::PreprocessingConfig& preprocessing_config,
      format_context& ctx
   ) -> decltype(ctx.out());
};
