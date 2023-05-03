#include "silo/prepare_dataset.h"

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for_each.h>
#include <unordered_set>

#include "silo/common/fasta_reader.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/database.h"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"

[[maybe_unused]] void silo::pruneMetadata(
   std::istream& metadata_in,
   silo::FastaReader& sequences_in,
   std::ostream& metadata_out
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

   std::string header;
   if (!getline(metadata_in, header, '\n')) {
      throw silo::PreprocessingException("Did not find header in metadata file");
   }
   metadata_out << header << "\n";

   while (true) {
      std::string key;
      std::string rest;
      if (!getline(metadata_in, key, '\t')) {
         break;
      }

      if (found_primary_keys.contains(key)) {
         if (!getline(metadata_in, rest)) {
            break;
         }
         found_metadata_count++;
         metadata_out << key << "\t" << rest << "\n";
      } else {
         metadata_in.ignore(LONG_MAX, '\n');
      }
   }

   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);
}

[[maybe_unused]] void silo::pruneSequences(
   std::istream& metadata_in,
   silo::FastaReader& sequences_in,
   std::ostream& sequences_out
) {
   SPDLOG_INFO("Pruning sequences");

   std::unordered_set<std::string> primary_keys;
   uint32_t found_metadata_count = 0;
   {
      std::string header;
      if (!getline(metadata_in, header, '\n')) {
         throw silo::PreprocessingException("Metadata file is emtpy. At least Header is expected.");
      }

      while (true) {
         std::string key;
         if (!getline(metadata_in, key, '\t')) {
            break;
         }
         metadata_in.ignore(LONG_MAX, '\n');
         primary_keys.insert(key);
         found_metadata_count++;
      }
   }
   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);

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

std::unordered_map<std::string, std::string> partitionMetadataFile(
   std::istream& meta_in,
   const std::string& output_prefix,
   const silo::PangoLineageAliasLookup& alias_key,
   const std::string& metadata_file_extension,
   std::unordered_map<std::string, std::string>& pango_to_chunk,
   const std::vector<std::string>& chunk_names
) {
   std::unordered_map<std::string, std::string> key_to_chunk;

   SPDLOG_INFO("partitioning metadata file to {}", output_prefix);

   std::string header;
   if (!getline(meta_in, header, '\n')) {
      throw silo::PreprocessingException("No header file in meta input.");
   }

   std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_meta_ostream;
   for (const std::string& chunk_name : chunk_names) {
      const std::string chunk_sequence_filename =
         std::string(output_prefix).append(chunk_name).append(metadata_file_extension);
      auto out = make_unique<std::ofstream>(chunk_sequence_filename);
      chunk_to_meta_ostream[chunk_name] = std::move(out);
      *chunk_to_meta_ostream[chunk_name] << header << '\n';
   }

   while (true) {
      std::string key;
      std::string pango_lineage_raw;
      std::string rest;
      if (!getline(meta_in, key, '\t')) {
         break;
      }
      if (!getline(meta_in, pango_lineage_raw, '\t')) {
         break;
      }
      if (!getline(meta_in, rest, '\n')) {
         break;
      }

      std::string const pango_lineage = alias_key.resolvePangoLineageAlias(pango_lineage_raw);

      std::string const chunk = pango_to_chunk[pango_lineage];
      *chunk_to_meta_ostream[chunk] << key << '\t' << pango_lineage << '\t' << rest << '\n';

      // Now save the chunk where the key will go for the sequence partitioning
      key_to_chunk[key] = chunk;
   }
   return key_to_chunk;
}

void partitionSequenceFile(
   silo::FastaReader& sequence_in,
   const std::string& output_prefix,
   const std::string& sequence_file_extension,
   std::vector<std::string>& chunk_names,
   std::unordered_map<std::string, std::string>& key_to_chunk
) {
   SPDLOG_INFO("partitioning sequences file to {}", output_prefix);

   std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_seq_ostream;
   for (const std::string& chunk_name : chunk_names) {
      const std::string chunk_sequence_filename =
         std::string(output_prefix).append(chunk_name).append(sequence_file_extension);
      auto out = make_unique<std::ofstream>(chunk_sequence_filename);
      chunk_to_seq_ostream[chunk_name] = std::move(out);
   }
   SPDLOG_DEBUG("Created file streams for {}", output_prefix);

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
            "Key in metadata and sequences did not match " + key + "."
         );
      }

      std::string const chunk = key_to_chunk[key];
      *chunk_to_seq_ostream[chunk] << '>' << key << '\n' << genome << '\n';
   }
}

void silo::partitionSequences(
   const preprocessing::Partitions& partitions,
   std::istream& meta_in,
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

void sortChunk(
   std::istream& meta_in,
   silo::FastaReader& sequence_in,
   std::ostream& meta_out,
   std::ostream& sequence_out,
   PartitionChunk chunk
) {
   const std::string chunk_str =
      'P' + std::to_string(chunk.part) + '_' + 'C' + std::to_string(chunk.chunk);

   std::unordered_map<std::string, time_t> key_to_date;

   {
      struct MetaLine {
         std::string key;
         std::string pango;
         time_t date;
         std::string date_str;
         std::string rest;
      };

      std::vector<MetaLine> lines;
      lines.reserve(chunk.size);

      // Ignore Header
      std::string header;
      if (!getline(meta_in, header, '\n')) {
         throw silo::PreprocessingException("Did not find header in metadata file.");
      }
      while (true) {
         std::string key;
         std::string pango_lineage;
         std::string date_str;
         std::string rest;
         if (!getline(meta_in, key, '\t')) {
            break;
         }
         if (!getline(meta_in, pango_lineage, '\t')) {
            break;
         }
         if (!getline(meta_in, date_str, '\t')) {
            break;
         }
         if (!getline(meta_in, rest, '\n')) {
            break;
         }

         struct std::tm time_struct {};
         std::istringstream time_stream(date_str);
         time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
         std::time_t const date_time = mktime(&time_struct);

         lines.push_back(MetaLine{key, pango_lineage, date_time, date_str, rest});

         key_to_date[key] = date_time;
      }

      auto sorter = [](const MetaLine& line1, const MetaLine& line2) {
         return line1.date < line2.date;
      };
      std::sort(lines.begin(), lines.end(), sorter);

      meta_out << header << '\n';

      for (const MetaLine& line : lines) {
         meta_out << line.key << '\t' << line.pango << '\t' << line.date_str << '\t' << line.rest
                  << '\n';
      }
   }

   {
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
         time_t const date = key_to_date[key];
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

      SPDLOG_TRACE("Calculated postitions for every sequence {}", chunk_str);

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
}

void silo::sortChunks(
   const preprocessing::Partitions& partitions,
   const std::string& input_prefix,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
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
      silo::InputStreamWrapper const meta_in(file_name + metadata_file_extension);
      std::ofstream sequence_out(
         output_prefix + silo::buildChunkName(chunk.part, chunk.chunk) + sequence_file_extension
      );
      std::ofstream meta_out(
         output_prefix + silo::buildChunkName(chunk.part, chunk.chunk) + metadata_file_extension
      );
      sortChunk(meta_in.getInputStream(), sequence_in, meta_out, sequence_out, chunk);
   });
}
