#include "silo/prepare_dataset.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <oneapi/tbb/parallel_for_each.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <duckdb.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/date.h"
#include "silo/common/fasta_reader.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/pango_lineage.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"

namespace {

constexpr std::string_view ZSTDFASTA_EXTENSION(".zstdfasta");
constexpr std::string_view FASTA_EXTENSION(".fasta");

std::string joinFilenames(
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>& tuples,
   const std::string& join
) {
   std::vector<std::string> quoted;
   quoted.resize(tuples.size());
   std::transform(
      tuples.begin(),
      tuples.end(),
      quoted.begin(),
      [](const std::pair<silo::preprocessing::PartitionChunk, std::filesystem::path>& entry) {
         return "'" + entry.second.string() + "'";
      }
   );
   return boost::algorithm::join(quoted, join);
}

std::unordered_map<silo::preprocessing::PartitionChunk, silo::ZstdFastaWriter>
getSequenceWritersForChunks(
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      partition_filenames,
   std::string_view reference_genome
) {
   std::unordered_map<silo::preprocessing::PartitionChunk, silo::ZstdFastaWriter>
      chunk_to_seq_ostream;
   for (const auto& [partition_chunk, filename] : partition_filenames) {
      SPDLOG_TRACE("Sequence Writer is now inserted with filename {}", filename.string());
      chunk_to_seq_ostream.insert(
         {partition_chunk, silo::ZstdFastaWriter(filename, reference_genome)}
      );
   }
   return chunk_to_seq_ostream;
}

void writeSequenceChunks(
   silo::FastaReader& sequence_in,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& key_to_chunk,
   std::unordered_map<silo::preprocessing::PartitionChunk, silo::ZstdFastaWriter>&
      chunk_to_seq_ostream
) {
   std::optional<std::string> key;
   std::string genome;
   while (true) {
      key = sequence_in.next(genome);
      if (!key.has_value()) {
         break;
      }
      if (!key_to_chunk.contains(*key)) {
         SPDLOG_INFO(
            "Fasta key '" + *key + "' was not present in keys in metadata. Ignoring sequence."
         );
      } else {
         const auto& chunk = key_to_chunk.at(*key);
         chunk_to_seq_ostream.at(chunk).write(*key, genome);
      }
   }
}

void writeSequenceChunks(
   silo::ZstdFastaReader& sequence_in,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& key_to_chunk,
   std::unordered_map<silo::preprocessing::PartitionChunk, silo::ZstdFastaWriter>&
      chunk_to_seq_ostream
) {
   std::optional<std::string> key;
   std::string genome;
   while (true) {
      key = sequence_in.nextCompressed(genome);
      if (!key.has_value()) {
         break;
      }
      if (!key_to_chunk.contains(*key)) {
         SPDLOG_INFO(
            "Fasta key '" + *key + "' was not present in keys in metadata. Ignoring sequence."
         );
      } else {
         const auto& chunk = key_to_chunk.at(*key);
         chunk_to_seq_ostream.at(chunk).writeRaw(*key, genome);
      }
   }
}

void partitionSequenceFile(
   const std::filesystem::path& sequence_filename_without_extension,
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      partition_filenames,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& key_to_chunk,
   std::string_view reference_sequence
) {
   auto chunk_to_seq_writer = getSequenceWritersForChunks(partition_filenames, reference_sequence);

   const std::filesystem::path sequence_file_zstdfasta =
      sequence_filename_without_extension.string() + std::string(ZSTDFASTA_EXTENSION);
   const std::filesystem::path sequence_file_fasta =
      sequence_filename_without_extension.string() + std::string(ZSTDFASTA_EXTENSION);

   if (std::filesystem::exists(sequence_file_zstdfasta)) {
      silo::ZstdFastaReader sequence_in(sequence_file_zstdfasta, reference_sequence);

      writeSequenceChunks(sequence_in, key_to_chunk, chunk_to_seq_writer);
   } else if (std::filesystem::exists(sequence_file_fasta)) {
      silo::FastaReader sequence_in(sequence_file_fasta);

      writeSequenceChunks(sequence_in, key_to_chunk, chunk_to_seq_writer);
   } else {
      throw silo::PreprocessingException(
         "Did not find the input file with filename " + sequence_file_zstdfasta.string() + " or " +
         sequence_file_fasta.string()
      );
   }
}

void copySequenceFile(silo::FastaReader& sequence_in, silo::ZstdFastaWriter& sequence_out) {
   std::optional<std::string> key;
   std::string genome;
   while (true) {
      key = sequence_in.next(genome);
      if (!key.has_value()) {
         break;
      }
      sequence_out.write(*key, genome);
   }
}
}  // namespace

void silo::partitionData(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partitions,
   const PangoLineageAliasLookup& alias_key,
   const std::string& primary_key_field,
   const std::string& partition_by_field,
   const ReferenceGenomes& reference_genomes
) {
   const std::filesystem::path metadata_filename = preprocessing_config.getMetadataInputFilename();

   auto metadata_partition_filenames =
      preprocessing_config.getMetadataPartitionFilenames(partitions);

   auto key_to_chunk = partitionMetadataFile(
      metadata_filename,
      metadata_partition_filenames,
      partitions.getPangoToChunk(),
      alias_key,
      primary_key_field,
      partition_by_field
   );

   for (const auto& [nuc_name, reference_genome] : reference_genomes.raw_nucleotide_sequences) {
      const std::filesystem::path sequence_filename_without_extension =
         preprocessing_config.getNucFilenameNoExtension(nuc_name).string();

      auto partition_filenames =
         preprocessing_config.getNucPartitionFilenames(nuc_name, partitions);

      SPDLOG_INFO(
         "partitioning nucleotide sequences from {}[{}, {}] to [{}]",
         sequence_filename_without_extension.string(),
         FASTA_EXTENSION,
         ZSTDFASTA_EXTENSION,
         joinFilenames(partition_filenames, ",")
      );

      partitionSequenceFile(
         sequence_filename_without_extension, partition_filenames, key_to_chunk, reference_genome
      );
   }

   for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
      const std::filesystem::path sequence_filename_without_extension =
         preprocessing_config.getGeneFilenameNoExtension(aa_name);

      auto partition_filenames =
         preprocessing_config.getGenePartitionFilenames(aa_name, partitions);

      SPDLOG_INFO(
         "partitioning amino acid sequences from {}[{}, {}] to [{}]",
         sequence_filename_without_extension.string(),
         FASTA_EXTENSION,
         ZSTDFASTA_EXTENSION,
         joinFilenames(partition_filenames, ",")
      );

      partitionSequenceFile(
         sequence_filename_without_extension, partition_filenames, key_to_chunk, reference_sequence
      );
   }
   SPDLOG_INFO("Finished partitioning");
}

void silo::copyDataToPartitionDirectory(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const ReferenceGenomes& reference_genomes
) {
   const std::filesystem::path metadata_filename = preprocessing_config.getMetadataInputFilename();

   auto metadata_partition_filename = preprocessing_config.getMetadataPartitionFilename(0, 0);

   std::filesystem::copy(
      metadata_filename,
      metadata_partition_filename,
      std::filesystem::copy_options::overwrite_existing
   );

   for (const auto& [nuc_name, reference_genome] : reference_genomes.raw_nucleotide_sequences) {
      const std::filesystem::path sequence_out_filename =
         preprocessing_config.getNucPartitionFilename(nuc_name, 0, 0);

      const std::filesystem::path sequence_in_filename_no_extension =
         preprocessing_config.getNucFilenameNoExtension(nuc_name);
      const std::filesystem::path sequence_in_filename_zstdfasta =
         sequence_in_filename_no_extension.string() + std::string(ZSTDFASTA_EXTENSION);
      const std::filesystem::path sequence_in_filename_fasta =
         sequence_in_filename_no_extension.string() + std::string(FASTA_EXTENSION);

      if (std::filesystem::exists(sequence_in_filename_zstdfasta)) {
         SPDLOG_INFO(
            "copying nucleotide sequences from {} to {}",
            sequence_in_filename_zstdfasta.string(),
            sequence_out_filename.string()
         );
         std::filesystem::copy(
            sequence_in_filename_zstdfasta,
            sequence_out_filename,
            std::filesystem::copy_options::overwrite_existing
         );
      } else if (std::filesystem::exists(sequence_in_filename_fasta)) {
         FastaReader sequence_in(sequence_in_filename_fasta);

         ZstdFastaWriter sequence_out(sequence_out_filename, reference_genome);

         SPDLOG_INFO(
            "copying and compressing nucleotide sequences from {} to {}",
            sequence_in_filename_fasta.string(),
            sequence_out_filename.string()
         );

         copySequenceFile(sequence_in, sequence_out);
      } else {
         throw silo::PreprocessingException(
            "Did not find the input file with filename " + sequence_in_filename_zstdfasta.string() +
            " or " + sequence_in_filename_fasta.string()
         );
      }
   }

   for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
      const std::filesystem::path sequence_out_filename =
         preprocessing_config.getGenePartitionFilename(aa_name, 0, 0);

      const std::filesystem::path sequence_in_filename_no_extension =
         preprocessing_config.getGeneFilenameNoExtension(aa_name);
      const std::filesystem::path sequence_in_filename_zstdfasta =
         sequence_in_filename_no_extension.string() + std::string(ZSTDFASTA_EXTENSION);
      const std::filesystem::path sequence_in_filename_fasta =
         sequence_in_filename_no_extension.string() + std::string(FASTA_EXTENSION);

      if (std::filesystem::exists(sequence_in_filename_zstdfasta)) {
         SPDLOG_INFO(
            "copying amino acid sequences from {} to {}",
            sequence_in_filename_zstdfasta.string(),
            sequence_out_filename.string()
         );

         std::filesystem::copy(
            sequence_in_filename_zstdfasta,
            sequence_out_filename,
            std::filesystem::copy_options::overwrite_existing
         );
      } else if (std::filesystem::exists(sequence_in_filename_fasta)) {
         FastaReader sequence_in(sequence_in_filename_fasta);

         ZstdFastaWriter sequence_out(sequence_out_filename, reference_sequence);

         SPDLOG_INFO(
            "copying and compressing amino acid sequences from {} to {}",
            sequence_in_filename_fasta.string(),
            sequence_out_filename.string()
         );

         copySequenceFile(sequence_in, sequence_out);
      } else {
         throw silo::PreprocessingException(
            "Did not find the input file with filename " + sequence_in_filename_zstdfasta.string() +
            " or " + sequence_in_filename_fasta.string()
         );
      }
   }
   SPDLOG_INFO("Finished copying to partition directory");
}
