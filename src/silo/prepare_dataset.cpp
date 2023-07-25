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
   const silo::config::DatabaseConfig& database_config
) {
   std::unordered_map<std::string, silo::preprocessing::PartitionChunk>
      primary_key_to_sequence_partition_chunk;
   for (auto& row : metadata_reader) {
      const std::string primary_key = row[database_config.schema.primary_key].get();
      const silo::common::RawPangoLineage raw_pango_lineage{
         row[database_config.schema.partition_by].get()};
      const silo::common::UnaliasedPangoLineage pango_lineage =
         alias_key.unaliasPangoLineage(raw_pango_lineage);
      row[database_config.schema.partition_by] = csv::CSVField{pango_lineage.value};

      const auto partition_chunk = pango_to_chunk.at(pango_lineage.value);

      chunk_to_metadata_writers[partition_chunk]->writeRow(row);

      primary_key_to_sequence_partition_chunk[primary_key] = partition_chunk;
   }

   return primary_key_to_sequence_partition_chunk;
}

std::unordered_map<std::string, silo::preprocessing::PartitionChunk> partitionMetadataFile(
   const std::filesystem::path& metadata_filename,
   const std::unordered_map<silo::preprocessing::PartitionChunk, std::filesystem::path>&
      metadata_partition_filenames,
   const std::unordered_map<std::string, silo::preprocessing::PartitionChunk>& pango_to_chunk,
   const silo::PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config
) {
   SPDLOG_INFO("partitioning metadata file {}", metadata_filename.string());

   silo::preprocessing::MetadataReader metadata_reader(metadata_filename);

   auto chunk_to_metadata_writers =
      getMetadataWritersForChunks(metadata_partition_filenames, metadata_reader.reader);

   return writeMetadataChunks(
      alias_key, pango_to_chunk, metadata_reader.reader, chunk_to_metadata_writers, database_config
   );
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
         throw silo::PreprocessingException(
            "Sequence key '" + *key + "' was not present in keys in metadata."
         );
      }

      const auto& chunk = key_to_chunk.at(*key);
      chunk_to_seq_ostream.at(chunk).write(*key, genome);
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

void silo::partitionData(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partitions,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config,
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
      database_config
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

std::unordered_map<std::string, silo::common::Date> sortMetadataFile(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const silo::preprocessing::PartitionChunk& chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   std::unordered_map<std::string, silo::common::Date> primary_key_to_date;

   struct RowWithDate {
      csv::CSVRow row;
      silo::common::Date date;
   };
   std::vector<RowWithDate> rows;
   rows.reserve(chunk.size);

   for (auto& row : metadata_reader.reader) {
      const auto primary_key = row[sort_chunk_config.primary_key_name].get();
      const auto date_str = row[sort_chunk_config.date_column_to_sort_by].get();

      const silo::common::Date date = silo::common::stringToDate(date_str);

      rows.push_back({row, date});

      primary_key_to_date[primary_key] = date;
   }

   std::sort(rows.begin(), rows.end(), [](const RowWithDate& line1, const RowWithDate& line2) {
      return line1.date < line2.date;
   });

   metadata_writer.writeHeader(metadata_reader.reader);

   for (const auto& row_with_date : rows) {
      metadata_writer.writeRow(row_with_date.row);
   }
   return primary_key_to_date;
}

void sortSequenceFile(
   silo::ZstdFastaReader& sequence_in,
   silo::ZstdFastaWriter& sequence_out,
   std::unordered_map<std::string, silo::common::Date>& primary_key_to_date
) {
   // Read file once, fill all dates, sort dates,
   // calculated target position for every genome
   // Reset gpointer, read file again, putting every genome at the correct position.
   // Write file to ostream

   struct KeyDatePair {
      std::string key;
      silo::common::Date date;
      uint32_t file_pos;
   };
   std::vector<KeyDatePair> key_date_pairs;
   uint32_t number_of_sequences = 0;
   std::optional<std::string> key;
   std::string compressed_genome;
   while (true) {
      key = sequence_in.nextCompressed(compressed_genome);
      if (!key.has_value()) {
         break;
      }
      silo::common::Date const date = primary_key_to_date[*key];
      key_date_pairs.emplace_back(KeyDatePair{*key, date, number_of_sequences++});
   }

   auto sorter = [](const KeyDatePair& date1, const KeyDatePair& date2) {
      return date1.date < date2.date;
   };
   std::sort(key_date_pairs.begin(), key_date_pairs.end(), sorter);

   std::vector<uint32_t> file_pos_to_sorted_pos(number_of_sequences);
   uint32_t number_of_sorted_files = 0;
   for (auto& key_date_pair : key_date_pairs) {
      file_pos_to_sorted_pos[key_date_pair.file_pos] = number_of_sorted_files++;
   }

   sequence_in.reset();

   constexpr uint32_t LINES_PER_SEQUENCE = 2;
   std::vector<std::string> lines_sorted(
      static_cast<uint64_t>(LINES_PER_SEQUENCE * number_of_sequences)
   );
   for (auto pos : file_pos_to_sorted_pos) {
      const uint64_t first_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * pos;
      const uint64_t second_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * pos + 1;
      auto sorted_key = sequence_in.nextCompressed(lines_sorted.at(second_line));
      if (!sorted_key) {
         SPDLOG_ERROR("Reached EOF too early.");
         return;
      }
      lines_sorted.at(first_line) = *sorted_key;
   }

   for (uint32_t sequence = 0; sequence < number_of_sequences; ++sequence) {
      const uint64_t first_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * sequence;
      const uint64_t second_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * sequence + 1;
      sequence_out.writeRaw(lines_sorted.at(first_line), lines_sorted.at(second_line));
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

   auto primary_key_to_date =
      sortMetadataFile(metadata_reader, metadata_writer, chunk, sort_chunk_config);

   SPDLOG_TRACE("Sorting sequences for partition {} chunk {}", chunk.partition, chunk.chunk);

   for (size_t nuc_idx = 0; nuc_idx < sequence_inputs.size(); ++nuc_idx) {
      sortSequenceFile(sequence_inputs[nuc_idx], sequence_outputs[nuc_idx], primary_key_to_date);
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
            sequence_outputs.emplace_back(output_filename, reference_sequence);
         }
         for (const auto& [aa_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
            const std::filesystem::path input_filename =
               preprocessing_config.getGenePartitionFilename(aa_name, partition, chunk);
            sequence_inputs.emplace_back(input_filename, reference_sequence);

            const std::filesystem::path output_filename =
               preprocessing_config.getGeneSortedPartitionFilename(aa_name, partition, chunk);
            sequence_outputs.emplace_back(output_filename, reference_sequence);
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
