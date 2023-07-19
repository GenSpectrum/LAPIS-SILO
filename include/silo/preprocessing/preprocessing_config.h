#ifndef SILO_PREPROCESSING_CONFIG_H
#define SILO_PREPROCESSING_CONFIG_H

#include <filesystem>
#include <string>
#include <unordered_map>

namespace silo::preprocessing {
struct PartitionChunk;
struct Partitions;
}  // namespace silo::preprocessing

#include <fmt/core.h>

namespace silo::preprocessing {

struct InputDirectory {
   std::string directory;
};

struct OutputDirectory {
   std::string directory;
};

struct MetadataFilename {
   std::string filename;
};

struct PangoLineageDefinitionFilename {
   std::string filename;
};

struct NucleotideSequencePrefix {
   std::string prefix;
};

struct GenePrefix {
   std::string prefix;
};

struct PartitionsFolder {
   std::string folder;
};

struct SortedPartitionsFolder {
   std::string folder;
};

struct SerializedStateFolder {
   std::string folder;
};

struct ReferenceGenomeFilename {
   std::string filename;
};

class PreprocessingConfig {
   friend class fmt::formatter<silo::preprocessing::PreprocessingConfig>;

   std::filesystem::path input_directory;
   std::filesystem::path pango_lineage_definition_file;
   std::filesystem::path metadata_file;
   std::filesystem::path partition_folder;
   std::filesystem::path sorted_partition_folder;
   std::filesystem::path serialization_folder;
   std::filesystem::path reference_genome_file;
   std::string nucleotide_sequence_prefix;
   std::string gene_prefix;

  public:
   explicit PreprocessingConfig();

   explicit PreprocessingConfig(
      const InputDirectory& input_directory_,
      const OutputDirectory& output_directory_,
      const MetadataFilename& metadata_filename_,
      const PangoLineageDefinitionFilename& pango_lineage_definition_filename_,
      const PartitionsFolder& partition_folder_,
      const SortedPartitionsFolder& sorted_partition_folder_,
      const SerializedStateFolder& serialization_folder_,
      const ReferenceGenomeFilename& reference_genome_filename_,
      const NucleotideSequencePrefix& nucleotide_sequence_prefix_,
      const GenePrefix& gene_prefix_
   );

   [[nodiscard]] std::filesystem::path getPangoLineageDefinitionFilename() const;

   [[nodiscard]] std::filesystem::path getReferenceGenomeFilename() const;

   [[nodiscard]] std::filesystem::path getMetadataInputFilename() const;

   [[nodiscard]] std::filesystem::path getMetadataPartitionFilename(
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::filesystem::path getMetadataSortedPartitionFilename(
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::filesystem::path getNucFilename(std::string_view nuc_name) const;

   [[nodiscard]] std::filesystem::path getNucPartitionFilename(
      std::string_view nuc_name,
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::filesystem::path getNucSortedPartitionFilename(
      std::string_view nuc_name,
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::filesystem::path getGeneFilename(std::string_view gene_name) const;

   [[nodiscard]] std::filesystem::path getGenePartitionFilename(
      std::string_view gene_name,
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::filesystem::path getGeneSortedPartitionFilename(
      std::string_view gene_name,
      uint32_t partition,
      uint32_t chunk
   ) const;

   [[nodiscard]] std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>
   getNucPartitionFilenames(
      std::string_view nuc_name,
      const silo::preprocessing::Partitions& partitions
   ) const;

   [[nodiscard]] std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>
   getGenePartitionFilenames(
      std::string_view gene_name,
      const silo::preprocessing::Partitions& partitions
   ) const;
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

#endif  // SILO_PREPROCESSING_CONFIG_H
