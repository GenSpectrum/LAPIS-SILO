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
#include <csv.hpp>

#include "silo/common/date.h"
#include "silo/common/fasta_reader.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/common/zstdfasta_writer.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"

const std::string ZSTDFASTA_EXTENSION(".zstdfasta");
const std::string TSV_EXTENSION(".tsv");

[[maybe_unused]] void silo::pruneMetadata(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::FastaReader& sequences_in,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const silo::config::DatabaseConfig& database_config
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

   metadata_writer.writeHeader(metadata_reader.reader);

   for (auto& row : metadata_reader.reader) {
      const auto key = row[database_config.schema.primary_key].get();

      if (found_primary_keys.contains(key)) {
         metadata_writer.writeRow(row);
      }
   }

   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);
}

[[maybe_unused]] void silo::pruneSequences(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::FastaReader& sequences_in,
   std::ostream& sequences_out,
   const silo::config::DatabaseConfig& database_config
) {
   SPDLOG_INFO("Pruning sequences");

   const auto primary_key_vector = metadata_reader.getColumn(database_config.schema.primary_key);
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
   const std::filesystem::path& output_folder,
   const std::vector<std::string>& chunk_names,
   const csv::CSVReader& metadata_reader
) {
   std::unordered_map<std::string, std::unique_ptr<silo::preprocessing::MetadataWriter>>
      chunk_to_metadata_writers;
   for (const std::string& chunk_name : chunk_names) {
      std::filesystem::path metadata_filename = output_folder;
      metadata_filename += chunk_name + TSV_EXTENSION;
      auto metadata_writer =
         std::make_unique<silo::preprocessing::MetadataWriter>(metadata_filename);

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
      chunk_to_metadata_writers,
   const silo::config::DatabaseConfig& database_config
) {
   std::unordered_map<std::string, std::string> primary_key_to_sequence_partition_chunk;
   for (auto& row : metadata_reader) {
      std::string const primary_key = row[database_config.schema.primary_key].get();
      std::string const pango_lineage =
         alias_key.resolvePangoLineageAlias(row[database_config.schema.partition_by].get());
      row[database_config.schema.partition_by] = csv::CSVField{pango_lineage};

      std::string const chunk = pango_to_chunk[pango_lineage];
      chunk_to_metadata_writers[chunk]->writeRow(row);

      primary_key_to_sequence_partition_chunk[primary_key] = chunk;
   }

   return primary_key_to_sequence_partition_chunk;
}

std::unordered_map<std::string, std::string> partitionMetadataFile(
   silo::preprocessing::MetadataReader& metadata_reader,
   const std::filesystem::path& output_folder,
   const silo::PangoLineageAliasLookup& alias_key,
   std::unordered_map<std::string, std::string>& pango_to_chunk,
   const std::vector<std::string>& chunk_names,
   const silo::config::DatabaseConfig& database_config
) {
   SPDLOG_INFO("partitioning metadata file to {}", output_folder.string());

   auto chunk_to_metadata_writers =
      getMetadataWritersForChunks(output_folder, chunk_names, metadata_reader.reader);

   return writeMetadataChunks(
      alias_key, pango_to_chunk, metadata_reader.reader, chunk_to_metadata_writers, database_config
   );
}

std::unordered_map<std::string, silo::ZstdFastaWriter> getSequenceWritersForChunks(
   const std::filesystem::path& output_folder,
   const std::vector<std::string>& chunk_names,
   const std::string& reference_genome
) {
   std::unordered_map<std::string, silo::ZstdFastaWriter> chunk_to_seq_ostream;
   for (const std::string& chunk_name : chunk_names) {
      std::filesystem::path sequence_filename = output_folder;
      sequence_filename += chunk_name + ZSTDFASTA_EXTENSION;
      chunk_to_seq_ostream.insert(
         {chunk_name, silo::ZstdFastaWriter(sequence_filename, reference_genome)}
      );
   }
   return chunk_to_seq_ostream;
}

void writeSequenceChunks(
   silo::FastaReader& sequence_in,
   std::unordered_map<std::string, std::string>& key_to_chunk,
   std::unordered_map<std::string, silo::ZstdFastaWriter>& chunk_to_seq_ostream
) {
   std::string key;
   std::string genome;
   while (sequence_in.next(key, genome)) {
      if (!key_to_chunk.contains(key)) {
         throw silo::PreprocessingException(
            "Sequence key '" + key + "' was not present in keys in metadata."
         );
      }

      std::string const chunk = key_to_chunk[key];
      chunk_to_seq_ostream.at(chunk).write(key, genome);
   }
}

void partitionSequenceFile(
   silo::FastaReader& sequence_in,
   const std::filesystem::path& output_folder,
   std::vector<std::string>& chunk_names,
   std::unordered_map<std::string, std::string>& key_to_chunk,
   const std::string& reference_sequence
) {
   SPDLOG_INFO("partitioning sequences file to {}", output_folder.string());

   auto chunk_to_seq_writer =
      getSequenceWritersForChunks(output_folder, chunk_names, reference_sequence);
   SPDLOG_DEBUG("Created file streams in folder {}", output_folder.string());

   writeSequenceChunks(sequence_in, key_to_chunk, chunk_to_seq_writer);
}

void silo::partitionData(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& input_folder,
   silo::preprocessing::MetadataReader& metadata_reader,
   const std::filesystem::path& output_folder,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config,
   const ReferenceGenomes& reference_genomes
) {
   std::unordered_map<std::string, std::string> pango_to_chunk;
   std::vector<std::string> chunk_names;
   for (uint32_t i = 0, limit = partitions.partitions.size(); i < limit; ++i) {
      const auto& part = partitions.partitions[i];
      for (uint32_t j = 0, limit2 = part.chunks.size(); j < limit2; ++j) {
         const auto& chunk = part.chunks[j];
         chunk_names.push_back(silo::buildChunkString(i, j));
         for (const auto& pango : chunk.pango_lineages) {
            pango_to_chunk[pango] = chunk_names.back();
         }
      }
   }

   auto key_to_chunk = partitionMetadataFile(
      metadata_reader, output_folder, alias_key, pango_to_chunk, chunk_names, database_config
   );

   for (const auto& [nuc_name, reference_genome] : reference_genomes.nucleotide_sequences) {
      std::filesystem::path sequence_filename = input_folder;
      sequence_filename += "nuc_" + nuc_name + ".fasta";
      FastaReader sequence_input(sequence_filename);

      std::filesystem::path nuc_folder = output_folder;
      nuc_folder += "nuc_" + nuc_name + std::filesystem::path::preferred_separator;

      create_directory(nuc_folder);

      partitionSequenceFile(
         sequence_input, nuc_folder, chunk_names, key_to_chunk, reference_genome
      );
   }

   for (const auto& [aa_name, reference_genome] : reference_genomes.aa_sequences) {
      std::filesystem::path sequence_filename = input_folder;
      sequence_filename += "gene_" + aa_name + ".fasta";
      FastaReader sequence_input(sequence_filename);

      std::filesystem::path aa_folder = output_folder;
      aa_folder += "gene_" + aa_name + std::filesystem::path::preferred_separator;

      create_directory(aa_folder);

      partitionSequenceFile(sequence_input, aa_folder, chunk_names, key_to_chunk, reference_genome);
   }

   SPDLOG_INFO("Finished partitioning to {}", output_folder.string());
}

struct PartitionChunk {
   uint32_t part;
   uint32_t chunk;
   uint32_t size;
};

std::unordered_map<std::string, silo::common::Date> sortMetadataFile(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const PartitionChunk& chunk,
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
   std::string key;
   std::string compressed_genome;
   while (sequence_in.nextCompressed(key, compressed_genome)) {
      silo::common::Date const date = primary_key_to_date[key];
      key_date_pairs.emplace_back(KeyDatePair{key, date, number_of_sequences++});
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
      if (!sequence_in.nextCompressed(lines_sorted.at(first_line), lines_sorted.at(second_line))) {
         SPDLOG_ERROR("Reached EOF too early.");
         return;
      }
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
   const PartitionChunk chunk,
   const silo::SortChunkConfig& sort_chunk_config
) {
   SPDLOG_TRACE("Sorting metadata for chunk " + silo::buildChunkString(chunk.part, chunk.chunk));

   auto primary_key_to_date =
      sortMetadataFile(metadata_reader, metadata_writer, chunk, sort_chunk_config);

   SPDLOG_TRACE("Sorting sequences for chunk " + silo::buildChunkString(chunk.part, chunk.chunk));

   for (size_t nuc_idx = 0; nuc_idx < sequence_inputs.size(); ++nuc_idx) {
      sortSequenceFile(sequence_inputs[nuc_idx], sequence_outputs[nuc_idx], primary_key_to_date);
   }

   SPDLOG_TRACE(
      "Finished all sorting for chunk " + silo::buildChunkString(chunk.part, chunk.chunk)
   );
}

void silo::sortChunks(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& input_folder,
   const std::filesystem::path& output_folder,
   const SortChunkConfig& sort_chunk_config,
   const ReferenceGenomes& reference_genomes
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
      std::vector<silo::ZstdFastaReader> sequence_inputs;
      std::vector<silo::ZstdFastaWriter> sequence_outputs;
      for (const auto& [nuc_name, reference_sequence] : reference_genomes.nucleotide_sequences) {
         std::filesystem::path input_filename = input_folder;
         input_filename += "nuc_" + nuc_name + std::filesystem::path::preferred_separator;
         input_filename += silo::buildChunkString(chunk.part, chunk.chunk) + ZSTDFASTA_EXTENSION;
         sequence_inputs.emplace_back(input_filename, reference_sequence);

         std::filesystem::path output_filename = output_folder;
         output_filename += "nuc_" + nuc_name + std::filesystem::path::preferred_separator;
         create_directory(output_filename);
         output_filename += silo::buildChunkString(chunk.part, chunk.chunk) + ZSTDFASTA_EXTENSION;
         sequence_outputs.emplace_back(output_filename, reference_sequence);
      }
      for (const auto& [aa_name, reference_sequence] : reference_genomes.aa_sequences) {
         std::filesystem::path input_filename = input_folder;
         input_filename += "gene_" + aa_name + std::filesystem::path::preferred_separator;
         input_filename += silo::buildChunkString(chunk.part, chunk.chunk) + ZSTDFASTA_EXTENSION;
         sequence_inputs.emplace_back(input_filename, reference_sequence);

         std::filesystem::path output_filename = output_folder;
         output_filename += "gene_" + aa_name + std::filesystem::path::preferred_separator;
         create_directory(output_filename);
         output_filename += silo::buildChunkString(chunk.part, chunk.chunk) + ZSTDFASTA_EXTENSION;
         sequence_outputs.emplace_back(output_filename, reference_sequence);
      }
      std::filesystem::path metadata_input = input_folder;
      metadata_input += silo::buildChunkString(chunk.part, chunk.chunk) + TSV_EXTENSION;
      silo::preprocessing::MetadataReader metadata_reader(metadata_input);
      std::filesystem::path metadata_output = output_folder;
      metadata_output += silo::buildChunkString(chunk.part, chunk.chunk) + TSV_EXTENSION;
      silo::preprocessing::MetadataWriter metadata_writer(metadata_output);
      sortChunk(
         metadata_reader,
         sequence_inputs,
         metadata_writer,
         sequence_outputs,
         chunk,
         sort_chunk_config
      );
   });
}
