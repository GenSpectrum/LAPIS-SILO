#include "silo/preprocessing/preprocessing_config.h"

#include <filesystem>
#include <functional>
#include <system_error>
#include <vector>

#include "silo/preprocessing/partition.h"

constexpr std::string_view ZSTDFASTA_EXTENSION(".zstdfasta");
constexpr std::string_view TSV_EXTENSION(".tsv");
constexpr std::string_view FASTA_EXTENSION(".fasta");

namespace {

std::string buildChunkString(uint32_t partition, uint32_t chunk) {
   return "P" + std::to_string(partition) + "_C" + std::to_string(chunk);
}

}  // namespace

namespace silo::preprocessing {

std::filesystem::path createOutputPath(
   const std::filesystem::path& output_directory,
   const std::string& folder
) {
   auto return_path = output_directory / folder;
   if (!std::filesystem::exists(return_path)) {
      std::filesystem::create_directory(return_path);
   }
   return return_path;
}

PreprocessingConfig::PreprocessingConfig() = default;

PreprocessingConfig::PreprocessingConfig(
   const InputDirectory& input_directory_,
   const IntermediateResultsDirectory& intermediate_results_directory_,
   const OutputDirectory& output_directory_,
   const MetadataFilename& metadata_filename_,
   const PangoLineageDefinitionFilename& pango_lineage_definition_filename_,
   const PartitionsFolder& partition_folder_,
   const SortedPartitionsFolder& sorted_partition_folder_,
   const ReferenceGenomeFilename& reference_genome_filename_,
   const NucleotideSequencePrefix& nucleotide_sequence_prefix_,
   const GenePrefix& gene_prefix_
) {
   input_directory = input_directory_.directory;
   if (!std::filesystem::exists(input_directory)) {
      throw std::filesystem::filesystem_error(
         input_directory.string() + " does not exist", std::error_code()
      );
   }

   metadata_file = input_directory / metadata_filename_.filename;
   if (pango_lineage_definition_filename_.filename.has_value()) {
      pango_lineage_definition_file =
         input_directory / pango_lineage_definition_filename_.filename.value();
   }
   reference_genome_file = input_directory / reference_genome_filename_.filename;

   if (!std::filesystem::exists(output_directory_.directory)) {
      std::filesystem::create_directory(output_directory_.directory);
   }
   this->output_directory = output_directory_.directory;

   const std::filesystem::path intermediate_results_directory(
      intermediate_results_directory_.directory
   );
   if (!std::filesystem::exists(intermediate_results_directory_.directory)) {
      std::filesystem::create_directory(intermediate_results_directory_.directory);
   }

   partition_folder = createOutputPath(intermediate_results_directory, partition_folder_.folder);
   sorted_partition_folder =
      createOutputPath(intermediate_results_directory, sorted_partition_folder_.folder);
   nucleotide_sequence_prefix = nucleotide_sequence_prefix_.prefix;
   gene_prefix = gene_prefix_.prefix;
}

std::filesystem::path PreprocessingConfig::getOutputDirectory() const {
   return output_directory;
}

std::optional<std::filesystem::path> PreprocessingConfig::getPangoLineageDefinitionFilename(
) const {
   return pango_lineage_definition_file;
}

std::filesystem::path PreprocessingConfig::getReferenceGenomeFilename() const {
   return reference_genome_file;
}

std::filesystem::path PreprocessingConfig::getMetadataInputFilename() const {
   return metadata_file;
}

std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> PreprocessingConfig::
   getMetadataPartitionFilenames(const silo::preprocessing::Partitions& partitions) const {
   std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> result;
   for (auto [partition, chunk, size] : partitions.getPartitionChunks()) {
      const std::filesystem::path filename = getMetadataPartitionFilename(partition, chunk);
      result.insert({{partition, chunk, size}, filename});
   }
   return result;
}

std::filesystem::path PreprocessingConfig::getMetadataPartitionFilename(
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path filename = partition_folder;
   filename /= buildChunkString(partition, chunk);
   filename += TSV_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getMetadataSortedPartitionFilename(
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path filename = sorted_partition_folder;
   filename /= buildChunkString(partition, chunk);
   filename += TSV_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucFilename(std::string_view nuc_name) const {
   std::filesystem::path filename = input_directory;
   filename /= nucleotide_sequence_prefix;
   filename += nuc_name;
   filename += FASTA_EXTENSION;
   return filename;
}

std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> PreprocessingConfig::
   getNucPartitionFilenames(
      std::string_view nuc_name,
      const silo::preprocessing::Partitions& partitions
   ) const {
   std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> result;
   for (auto [partition, chunk, size] : partitions.getPartitionChunks()) {
      const std::filesystem::path filename = getNucPartitionFilename(nuc_name, partition, chunk);
      result.insert({{partition, chunk, size}, filename});
   }
   return result;
}

std::filesystem::path PreprocessingConfig::getNucPartitionFilename(
   std::string_view nuc_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = nucleotide_sequence_prefix;
   folder += nuc_name;
   std::filesystem::path filename = createOutputPath(partition_folder, folder);
   filename /= buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucSortedPartitionFilename(
   std::string_view nuc_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = nucleotide_sequence_prefix;
   folder += nuc_name;
   std::filesystem::path filename = createOutputPath(sorted_partition_folder, folder);
   filename /= buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneFilename(std::string_view gene_name) const {
   std::filesystem::path filename = input_directory;
   filename /= gene_prefix;
   filename += gene_name;
   filename += FASTA_EXTENSION;
   return filename;
}

std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> PreprocessingConfig::
   getGenePartitionFilenames(
      std::string_view gene_name,
      const silo::preprocessing::Partitions& partitions
   ) const {
   std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> result;
   for (auto [partition, chunk, size] : partitions.getPartitionChunks()) {
      const std::filesystem::path filename = getGenePartitionFilename(gene_name, partition, chunk);
      result.insert({{partition, chunk, size}, filename});
   }
   return result;
}

std::filesystem::path PreprocessingConfig::getGenePartitionFilename(
   std::string_view gene_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = gene_prefix;
   folder += gene_name;
   std::filesystem::path filename = createOutputPath(partition_folder, folder);
   filename /= buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneSortedPartitionFilename(
   std::string_view gene_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = gene_prefix;
   folder += gene_name;
   std::filesystem::path filename = createOutputPath(sorted_partition_folder, folder);
   filename /= buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

}  // namespace silo::preprocessing

[[maybe_unused]] auto fmt::formatter<silo::preprocessing::PreprocessingConfig>::format(
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   fmt::format_context& ctx
) -> decltype(ctx.out()) {
   return format_to(
      ctx.out(),
      "{{ input directory: '{}', pango_lineage_definition_file: {}, output_directory: '{}', "
      "metadata_file: '{}', partition_folder: '{}', sorted_partition_folder: '{}', "
      "reference_genome_file: '{}',  gene_file_prefix: '{}',  "
      "nucleotide_sequence_file_prefix: '{}' }}",
      preprocessing_config.input_directory.string(),
      preprocessing_config.output_directory.string(),
      preprocessing_config.pango_lineage_definition_file.has_value()
         ? "'" + preprocessing_config.pango_lineage_definition_file->string() + "'"
         : "none",
      preprocessing_config.metadata_file.string(),
      preprocessing_config.partition_folder.string(),
      preprocessing_config.sorted_partition_folder.string(),
      preprocessing_config.reference_genome_file.string(),
      preprocessing_config.nucleotide_sequence_prefix,
      preprocessing_config.gene_prefix
   );
}
