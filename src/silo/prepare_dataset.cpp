#include "silo/prepare_dataset.h"

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for_each.h>
#include <unordered_set>

#include "silo/common/fasta_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/common/time.h"
#include "silo/database.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"

[[maybe_unused]] void silo::pruneMetadata(
   const std::filesystem::path& metadata_in,
   silo::FastaReader& sequences_in,
   silo::preprocessing::MetadataWriter& metadata_writer
) {
   SPDLOG_INFO("Pruning metadata");

   std::unordered_set<std::string> found_primary_keys;
   uint32_t found_sequences_count = 0;
   uint32_t found_metadata_count = 0;
   {
      std::string key;
      while (sequences_in.nextKey(key)) {
         found_primary_keys.insert(key);
         found_sequences_count++;
      }
   }

   SPDLOG_INFO("Finished reading sequences, found {} sequences", found_sequences_count);

   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(metadata_in);

   metadata_writer.writeHeader(metadata_reader);

   for (auto& row : metadata_reader) {
      const auto key = row[silo::preprocessing::COLUMN_NAME_PRIMARY_KEY].get();

      if (found_primary_keys.contains(key)) {
         metadata_writer.writeRow(row);
      }
   }

   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);
}

[[maybe_unused]] void silo::pruneSequences(
   const std::filesystem::path& metadata_in,
   silo::FastaReader& sequences_in,
   std::ostream& sequences_out
) {
   SPDLOG_INFO("Pruning sequences");

   const auto primary_key_vector = silo::preprocessing::MetadataReader::getColumn(
      metadata_in, silo::preprocessing::COLUMN_NAME_PRIMARY_KEY
   );
   const std::unordered_set<std::string> primary_keys(
      primary_key_vector.begin(), primary_key_vector.end()
   );

   SPDLOG_INFO("Finished reading metadata, found {} rows", primary_keys.size());

   uint32_t found_sequences_count = 0;
   {
      std::string key;
      std::string genome;
      while (sequences_in.next(key, genome)) {
         if (primary_keys.contains(key)) {
            found_sequences_count++;
            sequences_out << key << "\n" << genome << "\n";
         }
      }
   }
   SPDLOG_INFO("Finished reading sequences, found {} sequences", found_sequences_count);
}

std::unordered_map<std::string, std::unique_ptr<silo::preprocessing::MetadataWriter>>
getMetadataWritersForChunks(
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::vector<std::string>& chunk_names,
   const csv::CSVReader& metadata_reader
) {
   std::unordered_map<std::string, std::unique_ptr<silo::preprocessing::MetadataWriter>>
      chunk_to_metadata_writers;
   for (const std::string& chunk_name : chunk_names) {
      auto out_stream = std::make_unique<std::ofstream>(
         std::string(output_prefix).append(chunk_name).append(metadata_file_extension)
      );
      auto metadata_writer =
         std::make_unique<silo::preprocessing::MetadataWriter>(std::move(out_stream));

      metadata_writer->writeHeader(metadata_reader);

      chunk_to_metadata_writers[chunk_name] = std::move(metadata_writer);
   }
   return chunk_to_metadata_writers;
}

std::unordered_map<std::string, std::string> writeMetadataChunks(
   const silo::PangoLineageAliasLookup& alias_key,
   std::unordered_map<std::string, std::string>& pango_to_chunk,
   csv::CSVReader& metadata_reader,
   std::unordered_map<std::string, std::unique_ptr<silo::preprocessing::MetadataWriter>>&
      chunk_to_metadata_writers
) {
   std::unordered_map<std::string, std::string> primary_key_to_sequence_partition_chunk;
   for (auto& row : metadata_reader) {
      std::string const primary_key = row[silo::preprocessing::COLUMN_NAME_PRIMARY_KEY].get();
      std::string const pango_lineage = alias_key.resolvePangoLineageAlias(
         row[silo::preprocessing::COLUMN_NAME_PANGO_LINEAGE].get()
      );
      row[silo::preprocessing::COLUMN_NAME_PANGO_LINEAGE] = csv::CSVField{pango_lineage};

      std::string const chunk = pango_to_chunk[pango_lineage];
      chunk_to_metadata_writers[chunk]->writeRow(row);

      primary_key_to_sequence_partition_chunk[primary_key] = chunk;
   }

   return primary_key_to_sequence_partition_chunk;
}

std::unordered_map<std::string, std::string> partitionMetadataFile(
   const std::filesystem::path& meta_in,
   const std::string& output_prefix,
   const silo::PangoLineageAliasLookup& alias_key,
   const std::string& metadata_file_extension,
   std::unordered_map<std::string, std::string>& pango_to_chunk,
   const std::vector<std::string>& chunk_names
) {
   SPDLOG_INFO("partitioning metadata file to {}", output_prefix);

   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(meta_in);

   auto chunk_to_metadata_writers = getMetadataWritersForChunks(
      output_prefix, metadata_file_extension, chunk_names, metadata_reader
   );

   return writeMetadataChunks(
      alias_key, pango_to_chunk, metadata_reader, chunk_to_metadata_writers
   );
}

std::unordered_map<std::string, std::unique_ptr<std::ostream>> getSequenceOutStreamsForChunks(
   const std::string& output_prefix,
   const std::string& sequence_file_extension,
   const std::vector<std::string>& chunk_names
) {
   std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_seq_ostream;
   for (const std::string& chunk_name : chunk_names) {
      const std::string chunk_sequence_filename =
         std::string(output_prefix).append(chunk_name).append(sequence_file_extension);
      auto out = make_unique<std::ofstream>(chunk_sequence_filename);
      chunk_to_seq_ostream[chunk_name] = std::move(out);
   }
   return chunk_to_seq_ostream;
}

void writeSequenceChunks(
   silo::FastaReader& sequence_in,
   std::unordered_map<std::string, std::string>& key_to_chunk,
   std::unordered_map<std::string, std::unique_ptr<std::ostream>>& chunk_to_seq_ostream
) {
   std::string key;
   std::string genome;
   while (sequence_in.next(key, genome)) {
      if (genome.length() != silo::GENOME_LENGTH) {
         throw silo::PreprocessingException(
            "Genome didn't have expected length " + std::to_string(silo::GENOME_LENGTH) + " (was " +
            std::to_string(genome.length()) + ")."
         );
      }
      if (!key_to_chunk.contains(key)) {
         throw silo::PreprocessingException(
            "Sequence key '" + key + "' was not present in keys in metadata."
         );
      }

      std::string const chunk = key_to_chunk[key];
      *chunk_to_seq_ostream[chunk] << '>' << key << '\n' << genome << '\n';
   }
}

void partitionSequenceFile(
   silo::FastaReader& sequence_in,
   const std::string& output_prefix,
   const std::string& sequence_file_extension,
   std::vector<std::string>& chunk_names,
   std::unordered_map<std::string, std::string>& key_to_chunk
) {
   SPDLOG_INFO("partitioning sequences file to {}", output_prefix);

   auto chunk_to_seq_ostream =
      getSequenceOutStreamsForChunks(output_prefix, sequence_file_extension, chunk_names);
   SPDLOG_DEBUG("Created file streams for {}", output_prefix);

   writeSequenceChunks(sequence_in, key_to_chunk, chunk_to_seq_ostream);
}

void silo::partitionSequences(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& meta_in,
   silo::FastaReader& sequence_in,
   const std::string& output_prefix,
   const PangoLineageAliasLookup& alias_key,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
) {
   std::unordered_map<std::string, std::string> pango_to_chunk;
   std::vector<std::string> chunk_names;
   for (unsigned i = 0, limit = partitions.partitions.size(); i < limit; ++i) {
      const auto& part = partitions.partitions[i];
      for (unsigned j = 0, limit2 = part.chunks.size(); j < limit2; ++j) {
         const auto& chunk = part.chunks[j];
         chunk_names.push_back(silo::buildChunkName(i, j));
         for (const auto& pango : chunk.pango_lineages) {
            pango_to_chunk[pango] = chunk_names.back();
         }
      }
   }

   auto key_to_chunk = partitionMetadataFile(
      meta_in, output_prefix, alias_key, metadata_file_extension, pango_to_chunk, chunk_names
   );

   partitionSequenceFile(
      sequence_in, output_prefix, sequence_file_extension, chunk_names, key_to_chunk
   );

   SPDLOG_INFO("Finished partitioning to {}", output_prefix);
}

struct PartitionChunk {
   uint32_t part;
   uint32_t chunk;
   uint32_t size;
};

std::unordered_map<std::string, time_t> sortMetadataFile(
   const std::filesystem::path& meta_in,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const PartitionChunk& chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   std::unordered_map<std::string, time_t> primary_key_to_date;

   auto metadata_reader = silo::preprocessing::MetadataReader::getReader(meta_in);

   struct RowWithDate {
      csv::CSVRow row;
      time_t date;
   };
   std::vector<RowWithDate> rows;
   rows.reserve(chunk.size);

   for (auto& row : metadata_reader) {
      const auto primary_key = row[sort_chunk_config.primary_key_name].get();
      const auto date_str = row[sort_chunk_config.date_column_to_sort_by].get();

      const auto date_time = silo::common::mapToTime(date_str);

      rows.push_back({row, date_time});

      primary_key_to_date[primary_key] = date_time;
   }

   std::sort(rows.begin(), rows.end(), [](const RowWithDate& line1, const RowWithDate& line2) {
      return line1.date < line2.date;
   });

   metadata_writer.writeHeader(metadata_reader);

   for (const auto& row_with_date : rows) {
      metadata_writer.writeRow(row_with_date.row);
   }
   return primary_key_to_date;
}

void sortSequenceFile(
   silo::FastaReader& sequence_in,
   std::ostream& sequence_out,
   const PartitionChunk& chunk,
   std::unordered_map<std::string, time_t>& primary_key_to_date
) {
   const std::string chunk_str =
      'P' + std::to_string(chunk.part) + '_' + 'C' + std::to_string(chunk.chunk);

   // Now:
   // Read file once, fill all dates, sort dates,
   // calculated target position for every genome
   // Reset gpointer, read file again, putting every genome at the correct position.
   // Write file to ostream

   struct KeyDatePair {
      std::string key;
      time_t date;
      uint32_t file_pos;
   };
   std::vector<KeyDatePair> key_date_pairs;
   key_date_pairs.reserve(chunk.size);
   uint32_t number_of_sequences = 0;
   std::string key;
   std::string genome;
   while (sequence_in.next(key, genome)) {
      time_t const date = primary_key_to_date[key];
      key_date_pairs.emplace_back(KeyDatePair{key, date, number_of_sequences++});
   }

   SPDLOG_TRACE("Finished first run for chunk {}", chunk_str);

   auto sorter = [](const KeyDatePair& date1, const KeyDatePair& date2) {
      return date1.date < date2.date;
   };
   std::sort(key_date_pairs.begin(), key_date_pairs.end(), sorter);

   SPDLOG_TRACE("Sorted first run for partition {}", chunk_str);

   std::vector<uint32_t> file_pos_to_sorted_pos(number_of_sequences);
   unsigned number_of_sorted_files = 0;
   for (auto& key_date_pair : key_date_pairs) {
      file_pos_to_sorted_pos[key_date_pair.file_pos] = number_of_sorted_files++;
   }

   SPDLOG_TRACE("Calculated positions for every sequence {}", chunk_str);

   sequence_in.reset();

   SPDLOG_TRACE("Reset file seek, now read second time, sorted {}", chunk_str);

   constexpr uint32_t LINES_PER_SEQUENCE = 2;
   std::vector<std::string> lines_sorted(
      static_cast<uint64_t>(LINES_PER_SEQUENCE * number_of_sequences)
   );
   for (auto pos : file_pos_to_sorted_pos) {
      const uint64_t first_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * pos;
      const uint64_t second_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * pos + 1;
      if (!sequence_in.next(lines_sorted.at(first_line), lines_sorted.at(second_line))) {
         SPDLOG_ERROR("Reached EOF too early.");
         return;
      }
   }

   for (uint32_t sequence = 0; sequence < number_of_sequences; ++sequence) {
      const uint64_t first_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * sequence;
      const uint64_t second_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * sequence + 1;
      sequence_out << '>' << lines_sorted.at(first_line) << '\n'
                   << lines_sorted.at(second_line) << '\n';
   }
}

void sortChunk(
   const std::filesystem::path& meta_in,
   silo::FastaReader& sequence_in,
   silo::preprocessing::MetadataWriter& metadata_writer,
   std::ostream& sequence_out,
   const PartitionChunk chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   auto primary_key_to_date = sortMetadataFile(meta_in, metadata_writer, chunk, sort_chunk_config);

   sortSequenceFile(sequence_in, sequence_out, chunk, primary_key_to_date);
}

void silo::sortChunks(
   const preprocessing::Partitions& partitions,
   const std::string& input_prefix,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension,
   const SortChunkConfig& sort_chunk_config
) {
   std::vector<PartitionChunk> all_chunks;
   for (uint32_t part_id = 0, limit = partitions.partitions.size(); part_id < limit; ++part_id) {
      const auto& part = partitions.partitions[part_id];
      for (uint32_t chunk_id = 0, limit2 = part.chunks.size(); chunk_id < limit2; ++chunk_id) {
         const auto& chunk = part.chunks[chunk_id];
         all_chunks.emplace_back(PartitionChunk{part_id, chunk_id, chunk.count_of_sequences});
      }
   }

   tbb::parallel_for_each(all_chunks.begin(), all_chunks.end(), [&](const PartitionChunk& chunk) {
      const auto& file_name = input_prefix + silo::buildChunkName(chunk.part, chunk.chunk);
      silo::FastaReader sequence_in(file_name + sequence_file_extension);
      std::ofstream sequence_out(
         output_prefix + silo::buildChunkName(chunk.part, chunk.chunk) + sequence_file_extension
      );
      silo::preprocessing::MetadataWriter meta_out(std::make_unique<std::ofstream>(
         output_prefix + silo::buildChunkName(chunk.part, chunk.chunk) + metadata_file_extension
      ));
      sortChunk(
         file_name + metadata_file_extension,
         sequence_in,
         meta_out,
         sequence_out,
         chunk,
         sort_chunk_config
      );
   });
}
