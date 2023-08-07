#include "silo/prepare_dataset.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <oneapi/tbb/parallel_for_each.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <csv.hpp>

#include "silo/common/date.h"
#include "silo/common/fasta_reader.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"

namespace {

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

}  // namespace

std::unordered_map<
   silo::preprocessing::PartitionChunk,
   std::unique_ptr<silo::preprocessing::MetadataWriter>>
getMetadataWritersForChunks(
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      metadata_partition_filenames,
   const csv::CSVReader& metadata_reader
) {
   std::unordered_map<
      silo::preprocessing::PartitionChunk,
      std::unique_ptr<silo::preprocessing::MetadataWriter>>
      chunk_to_metadata_writers;
   for (const auto& [partition_chunk, filename] : metadata_partition_filenames) {
      auto metadata_writer = std::make_unique<silo::preprocessing::MetadataWriter>(filename);

      metadata_writer->writeHeader(metadata_reader);

      chunk_to_metadata_writers[partition_chunk] = std::move(metadata_writer);
   }
   return chunk_to_metadata_writers;
}

std::unordered_map<std::string, silo::preprocessing::PartitionChunk> writeMetadataChunks(
   const silo::PangoLineageAliasLookup& alias_key,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& pango_to_chunk,
   csv::CSVReader& metadata_reader,
   std::unordered_map<
      silo::preprocessing::PartitionChunk,
      std::unique_ptr<silo::preprocessing::MetadataWriter>>& chunk_to_metadata_writers,
   const std::string& primary_key_field,
   const std::string& partition_by_field
) {
   std::unordered_map<std::string, silo::preprocessing::PartitionChunk>
      primary_key_to_sequence_partition_chunk;
   for (auto& row : metadata_reader) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(2));
      const std::string_view primary_key = row[primary_key_field].get_sv();
      const silo::common::RawPangoLineage raw_pango_lineage{
         row[partition_by_field].get<std::string>()};
      const silo::common::UnaliasedPangoLineage pango_lineage =
         alias_key.unaliasPangoLineage(raw_pango_lineage);

      const auto partition_chunk = pango_to_chunk.at(pango_lineage.value);

      chunk_to_metadata_writers[partition_chunk]->writeRow(row);

      primary_key_to_sequence_partition_chunk.emplace(primary_key, partition_chunk);
   }

   return primary_key_to_sequence_partition_chunk;
}

std::unordered_map<std::string, silo::preprocessing::PartitionChunk> partitionMetadataFile(
   const std::filesystem::path& metadata_filename,
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      metadata_partition_filenames,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& pango_to_chunk,
   const silo::PangoLineageAliasLookup& alias_key,
   const std::string& primary_key_field,
   const std::string& partition_by_field
) {
   SPDLOG_INFO("partitioning metadata file {}", metadata_filename.string());

   silo::preprocessing::MetadataReader metadata_reader(metadata_filename);

   auto chunk_to_metadata_writers =
      getMetadataWritersForChunks(metadata_partition_filenames, metadata_reader.reader);

   return writeMetadataChunks(
      alias_key,
      pango_to_chunk,
      metadata_reader.reader,
      chunk_to_metadata_writers,
      primary_key_field,
      partition_by_field
   );
}

void copyMetadataFile(
   csv::CSVReader& metadata_reader,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const std::string& primary_key_field
) {
   metadata_writer.writeHeader(metadata_reader);
   for (auto& row : metadata_reader) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(2));
      metadata_writer.writeRow(row);
   }
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

void partitionSequenceFile(
   const std::filesystem::path& sequence_filename,
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      partition_filenames,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& key_to_chunk,
   std::string_view reference_sequence
) {
   silo::FastaReader sequence_in(sequence_filename);

   auto chunk_to_seq_writer = getSequenceWritersForChunks(partition_filenames, reference_sequence);

   writeSequenceChunks(sequence_in, key_to_chunk, chunk_to_seq_writer);
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
      const std::filesystem::path sequence_filename = preprocessing_config.getNucFilename(nuc_name);

      auto partition_filenames =
         preprocessing_config.getNucPartitionFilenames(nuc_name, partitions);

      SPDLOG_INFO(
         "partitioning nucleotide sequences from {} to [{}]",
         sequence_filename.string(),
         joinFilenames(partition_filenames, ",")
      );

      partitionSequenceFile(sequence_filename, partition_filenames, key_to_chunk, reference_genome);
   }

   for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
      const std::filesystem::path sequence_filename = preprocessing_config.getGeneFilename(aa_name);

      auto partition_filenames =
         preprocessing_config.getGenePartitionFilenames(aa_name, partitions);

      SPDLOG_INFO(
         "partitioning amino acid sequences from {} to [{}]",
         sequence_filename.string(),
         joinFilenames(partition_filenames, ",")
      );

      partitionSequenceFile(
         sequence_filename, partition_filenames, key_to_chunk, reference_sequence
      );
   }
   SPDLOG_INFO("Finished partitioning");
}

void silo::copyDataToPartitionDirectory(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const std::string& primary_key_field,
   const ReferenceGenomes& reference_genomes
) {
   const std::filesystem::path metadata_filename = preprocessing_config.getMetadataInputFilename();
   auto metadata_reader = preprocessing::MetadataReader(metadata_filename);

   auto metadata_partition_filename = preprocessing_config.getMetadataPartitionFilename(0, 0);
   auto metadata_writer = preprocessing::MetadataWriter(metadata_partition_filename);

   copyMetadataFile(metadata_reader.reader, metadata_writer, primary_key_field);

   for (const auto& [nuc_name, reference_genome] : reference_genomes.raw_nucleotide_sequences) {
      const std::filesystem::path sequence_in_filename =
         preprocessing_config.getNucFilename(nuc_name);
      FastaReader sequence_in(sequence_in_filename);

      auto sequence_out_filename = preprocessing_config.getNucPartitionFilename(nuc_name, 0, 0);
      ZstdFastaWriter sequence_out(sequence_out_filename, reference_genome);

      SPDLOG_INFO(
         "copying and compressing nucleotide sequences from {} to {}",
         sequence_in_filename.string(),
         sequence_out_filename.string()
      );

      copySequenceFile(sequence_in, sequence_out);
   }

   for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
      const std::filesystem::path sequence_in_filename =
         preprocessing_config.getGeneFilename(aa_name);
      FastaReader sequence_in(sequence_in_filename);

      auto sequence_out_filename = preprocessing_config.getGenePartitionFilename(aa_name, 0, 0);
      ZstdFastaWriter sequence_out(sequence_out_filename, reference_sequence);

      SPDLOG_INFO(
         "copying and compressing amino acid sequences from {} to {}",
         sequence_in_filename.string(),
         sequence_out_filename.string()
      );

      copySequenceFile(sequence_in, sequence_out);
   }
   SPDLOG_INFO("Finished copying to partition directory");
}

std::unordered_map<std::string, size_t> sortMetadataFile(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const silo::preprocessing::PartitionChunk& chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   struct RowWithKeyDate {
      csv::CSVRow row;
      std::string primary_key;
      silo::common::Date date;

      RowWithKeyDate(csv::CSVRow& row, std::string&& primary_key, silo::common::Date date)
          : row(row),
            primary_key(primary_key),
            date(date) {}
   };
   std::vector<RowWithKeyDate> rows;
   rows.reserve(chunk.size);

   for (auto& row : metadata_reader.reader) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(1));

      std::string primary_key(row[sort_chunk_config.primary_key_name].get_sv());
      const std::string date_str(row[sort_chunk_config.date_column_to_sort_by].get_sv());

      const silo::common::Date date = silo::common::stringToDate(date_str);

      rows.emplace_back(row, std::move(primary_key), date);
   }

   std::sort(
      rows.begin(),
      rows.end(),
      [](const RowWithKeyDate& line1, const RowWithKeyDate& line2) {
         return line1.date < line2.date;
      }
   );

   metadata_writer.writeHeader(metadata_reader.reader);

   std::unordered_map<std::string, size_t> primary_key_to_position;

   size_t position = 0;
   for (const auto& row_with_date : rows) {
      primary_key_to_position[row_with_date.primary_key] = position++;
      metadata_writer.writeRow(row_with_date.row);
   }
   return primary_key_to_position;
}

void sortSequenceFile(
   silo::ZstdFastaReader& sequence_in,
   silo::ZstdFastaWriter& sequence_out,
   const std::unordered_map<std::string, size_t>& primary_key_to_position
) {
   std::vector<std::pair<std::string, std::string>> lines_sorted(primary_key_to_position.size());
   std::string compressed_genome;
   while (true) {
      auto sorted_key = sequence_in.nextCompressed(compressed_genome);
      if (sorted_key == std::nullopt) {
         break;
      }
      const size_t position = primary_key_to_position.at(*sorted_key);
      lines_sorted[position] = std::make_pair(*sorted_key, compressed_genome);
   }

   for (const auto& [key, compressed_sequence] : lines_sorted) {
      if (compressed_sequence.empty()) {
         SPDLOG_WARN("Do not have sequence for key: '" + key + "' falling back to default");
         sequence_out.writeDefault(key);
      } else {
         sequence_out.writeRaw(key, compressed_sequence);
      };
   }
}

void sortChunk(
   silo::preprocessing::MetadataReader& metadata_reader,
   std::vector<silo::ZstdFastaReader>& sequence_inputs,
   silo::preprocessing::MetadataWriter& metadata_writer,
   std::vector<silo::ZstdFastaWriter>& sequence_outputs,
   const silo::preprocessing::PartitionChunk chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   SPDLOG_TRACE("Sorting metadata for partition {} chunk {}", chunk.partition, chunk.chunk);

   auto primary_key_to_position =
      sortMetadataFile(metadata_reader, metadata_writer, chunk, sort_chunk_config);

   SPDLOG_TRACE("Sorting sequences for partition {} chunk {}", chunk.partition, chunk.chunk);

   for (size_t nuc_idx = 0; nuc_idx < sequence_inputs.size(); ++nuc_idx) {
      sortSequenceFile(
         sequence_inputs[nuc_idx], sequence_outputs[nuc_idx], primary_key_to_position
      );
   }

   SPDLOG_TRACE("Finished all sorting for partition {} chunk {}", chunk.partition, chunk.chunk);
}

void silo::sortChunks(
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partitions,
   const SortChunkConfig& sort_chunk_config,
   const ReferenceGenomes& reference_genomes
) {
   tbb::parallel_for_each(
      partitions.getPartitionChunks().begin(),
      partitions.getPartitionChunks().end(),
      [&](const preprocessing::PartitionChunk& partition_chunk) {
         const uint32_t partition = partition_chunk.partition;
         const uint32_t chunk = partition_chunk.chunk;
         std::vector<silo::ZstdFastaReader> sequence_inputs;
         std::vector<silo::ZstdFastaWriter> sequence_outputs;
         for (const auto& [nuc_name, reference_sequence] :
              reference_genomes.raw_nucleotide_sequences) {
            const std::filesystem::path input_filename =
               preprocessing_config.getNucPartitionFilename(nuc_name, partition, chunk);
            sequence_inputs.emplace_back(input_filename, reference_sequence);

            const std::filesystem::path output_filename =
               preprocessing_config.getNucSortedPartitionFilename(nuc_name, partition, chunk);
            sequence_outputs.emplace_back(
               output_filename,
               reference_sequence,
               std::string(
                  reference_sequence.length(), silo::nucleotideSymbolToChar(NUCLEOTIDE_SYMBOL::N)
               )
            );
         }
         for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
            const std::filesystem::path input_filename =
               preprocessing_config.getGenePartitionFilename(aa_name, partition, chunk);
            sequence_inputs.emplace_back(input_filename, reference_sequence);

            const std::filesystem::path output_filename =
               preprocessing_config.getGeneSortedPartitionFilename(aa_name, partition, chunk);
            sequence_outputs.emplace_back(
               output_filename,
               reference_sequence,
               std::string(reference_sequence.length(), silo::aaSymbolToChar(AA_SYMBOL::X))
            );
         }
         const std::filesystem::path metadata_input =
            preprocessing_config.getMetadataPartitionFilename(partition, chunk);
         silo::preprocessing::MetadataReader metadata_reader(metadata_input);
         const std::filesystem::path metadata_output =
            preprocessing_config.getMetadataSortedPartitionFilename(partition, chunk);
         silo::preprocessing::MetadataWriter metadata_writer(metadata_output);
         sortChunk(
            metadata_reader,
            sequence_inputs,
            metadata_writer,
            sequence_outputs,
            partition_chunk,
            sort_chunk_config
         );
      }
   );
}
