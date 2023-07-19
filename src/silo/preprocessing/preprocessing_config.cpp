#include "silo/preprocessing/preprocessing_config.h"

#include <silo/database.h>
#include <filesystem>
#include <system_error>

constexpr std::string_view ZSTDFASTA_EXTENSION(".zstdfasta");
constexpr std::string_view TSV_EXTENSION(".tsv");
constexpr std::string_view FASTA_EXTENSION(".fasta");

namespace silo::preprocessing {

std::filesystem::path createPath(
   const std::filesystem::path& directory,
   const std::string& filename
) {
   auto return_path = directory;
   return_path += filename;
   if (!std::filesystem::exists(return_path)) {
      throw std::filesystem::filesystem_error(
         return_path.relative_path().string() + " does not exist", std::error_code()
      );
   }
   return return_path;
}

std::filesystem::path createOutputPath(
   const std::filesystem::path& output_directory,
   const std::string& folder
) {
   auto return_path = output_directory;
   return_path += folder;
   if (!std::filesystem::exists(return_path)) {
      std::filesystem::create_directory(return_path);
   }
   return return_path;
}

PreprocessingConfig::PreprocessingConfig() = default;

PreprocessingConfig::PreprocessingConfig(
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
) {
   input_directory = input_directory_.directory;
   if (!std::filesystem::exists(input_directory)) {
      throw std::filesystem::filesystem_error(
         input_directory.string() + " does not exist", std::error_code()
      );
   }

   metadata_file = createPath(input_directory, metadata_filename_.filename);
   pango_lineage_definition_file =
      createPath(input_directory, pango_lineage_definition_filename_.filename);
   reference_genome_file = createPath(input_directory, reference_genome_filename_.filename);

   const std::filesystem::path output_directory(output_directory_.directory);
   if (!std::filesystem::exists(output_directory_.directory)) {
      std::filesystem::create_directory(output_directory_.directory);
   }

   partition_folder = createOutputPath(output_directory, partition_folder_.folder);
   sorted_partition_folder = createOutputPath(output_directory, sorted_partition_folder_.folder);
   serialization_folder = createOutputPath(output_directory, serialization_folder_.folder);
   nucleotide_sequence_prefix = nucleotide_sequence_prefix_.prefix;
   gene_prefix = gene_prefix_.prefix;
}

std::filesystem::path PreprocessingConfig::getPangoLineageDefinitionFilename() const {
   return pango_lineage_definition_file;
}

std::filesystem::path PreprocessingConfig::getReferenceGenomeFilename() const {
   return reference_genome_file;
}

std::filesystem::path PreprocessingConfig::getMetadataInputFilename() const {
   return metadata_file.string();
}

std::filesystem::path PreprocessingConfig::getMetadataPartitionFilename(
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path filename = partition_folder;
   filename += silo::buildChunkString(partition, chunk);
   filename += TSV_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getMetadataSortedPartitionFilename(
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path filename = sorted_partition_folder;
   filename += silo::buildChunkString(partition, chunk);
   filename += TSV_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucFilename(std::string_view nuc_name) const {
   std::filesystem::path filename = input_directory;
   filename += nucleotide_sequence_prefix;
   filename += nuc_name;
   filename += FASTA_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getNucPartitionFilename(
   std::string_view nuc_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = nucleotide_sequence_prefix;
   folder += nuc_name;
   folder += std::filesystem::path::preferred_separator;
   std::filesystem::path filename = createOutputPath(partition_folder, folder);
   filename += silo::buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> PreprocessingConfig::
   getNucPartitionFilenames(
      std::string_view nuc_name,
      const silo::preprocessing::Partitions& partitions
   ) const {
   std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path> result;
   for (auto [partition, chunk, size] : partitions.partition_chunks) {
      const std::filesystem::path filename = getNucPartitionFilename(nuc_name, partition, chunk);
      result.insert({{partition, chunk, size}, filename});
   }
   return result;
}

std::filesystem::path PreprocessingConfig::getNucSortedPartitionFilename(
   std::string_view nuc_name,
   uint32_t partition,
   uint32_t chunk
) const {
   std::filesystem::path folder = nucleotide_sequence_prefix;
   folder += nuc_name;
   folder += std::filesystem::path::preferred_separator;
   std::filesystem::path filename = createOutputPath(sorted_partition_folder, folder);
   filename += silo::buildChunkString(partition, chunk);
   filename += ZSTDFASTA_EXTENSION;
   return filename;
}

std::filesystem::path PreprocessingConfig::getGeneFilename(std::string_view gene_name) const {
   std::filesystem::path filename = input_directory;
   filename += gene_prefix;
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
   for (auto [partition, chunk, size] : partitions.partition_chunks) {
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
   folder += std::filesystem::path::preferred_separator;
   std::filesystem::path filename = createOutputPath(partition_folder, folder);
   filename += silo::buildChunkString(partition, chunk);
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
   folder += std::filesystem::path::preferred_separator;
   std::filesystem::path filename = createOutputPath(sorted_partition_folder, folder);
   filename += silo::buildChunkString(partition, chunk);
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
      "{{ input directory: '{}', pango_lineage_definition_file: '{}', "
      "metadata_file: '{}', partition_folder: '{}', sorted_partition_folder: '{}', "
      "serialization_folder: '{}', reference_genome_file: '{}',  gene_file_prefix: '{}',  "
      "nucleotide_sequence_file_prefix: '{}' }}",
      preprocessing_config.input_directory.string(),
      preprocessing_config.pango_lineage_definition_file.string(),
      preprocessing_config.metadata_file.string(),
      preprocessing_config.partition_folder.string(),
      preprocessing_config.sorted_partition_folder.string(),
      preprocessing_config.serialization_folder.string(),
      preprocessing_config.reference_genome_file.string(),
      preprocessing_config.nucleotide_sequence_prefix,
      preprocessing_config.gene_prefix
   );
}
