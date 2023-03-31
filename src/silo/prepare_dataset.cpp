#include "silo/prepare_dataset.h"

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for_each.h>
#include <unordered_set>

#include "silo/common/input_stream_wrapper.h"
#include "silo/database.h"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"

[[maybe_unused]] void silo::pruneMetadata(
   std::istream& metadata_in,
   std::istream& sequences_in,
   std::ostream& metadata_out
) {
   SPDLOG_INFO("Pruning metadata");

   std::unordered_set<uint64_t> epi_isl_ids;
   uint32_t found_sequences_count = 0;
   uint32_t found_metadata_count = 0;
   {
      while (true) {
         std::string epi_isl;
         if (!getline(sequences_in, epi_isl)) {
            break;
         }
         sequences_in.ignore(LONG_MAX, '\n');

         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE = 9;
         std::string const epi_isl_id = epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE);
         try {
            epi_isl_ids.insert(stoi(epi_isl_id));
            found_sequences_count++;
         } catch (const std::invalid_argument& exception) {
            throw silo::PreprocessingException(
               "Failed parsing EPI: " + epi_isl + " (sequence " +
               std::to_string(found_sequences_count) + "): " + exception.what()
            );
         }
      }
   }
   SPDLOG_INFO("Finished reading sequences, found {} sequences", found_sequences_count);

   {
      std::string header;
      if (!getline(metadata_in, header, '\n')) {
         throw silo::PreprocessingException("Did not find header in metadata file");
      }
      metadata_out << header << "\n";

      while (true) {
         std::string epi_isl;
         std::string rest;
         if (!getline(metadata_in, epi_isl, '\t')) {
            break;
         }

         std::string const tmp = epi_isl.substr(8);
         try {
            uint64_t const epi = stoi(tmp);
            if (epi_isl_ids.contains(epi)) {
               if (!getline(metadata_in, rest)) {
                  break;
               }
               found_metadata_count++;
               metadata_out << epi_isl << "\t" << rest << "\n";
            } else {
               metadata_in.ignore(LONG_MAX, '\n');
            }
         } catch (const std::invalid_argument& exception) {
            throw silo::PreprocessingException(
               "Failed parsing EPI: " + epi_isl + " (metadata row " +
               std::to_string(found_metadata_count) + "): " + exception.what()
            );
         }
      }
   }
   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);
}

[[maybe_unused]] void silo::pruneSequences(
   std::istream& metadata_in,
   std::istream& sequences_in,
   std::ostream& sequences_out
) {
   SPDLOG_INFO("Pruning sequences");

   std::unordered_set<uint64_t> set;
   uint32_t found_metadata_count = 0;
   {
      std::string header;
      if (!getline(metadata_in, header, '\n')) {
         throw silo::PreprocessingException("Metadata file is emtpy. At least Header is expected.");
      }

      while (true) {
         std::string epi_isl;
         if (!getline(metadata_in, epi_isl, '\t')) {
            break;
         }
         metadata_in.ignore(LONG_MAX, '\n');
         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL = 8;
         std::string const tmp = epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL);
         try {
            uint64_t const epi = stoi(tmp);
            set.insert(epi);
            found_metadata_count++;
         } catch (const std::invalid_argument& exception) {
            throw silo::PreprocessingException(
               "Failed parsing EPI: " + epi_isl + " (metadata row " +
               std::to_string(found_metadata_count) + "): " + exception.what()
            );
         }
      }
   }
   SPDLOG_INFO("Finished reading metadata, found {} rows", found_metadata_count);

   uint32_t found_sequences_count = 0;
   {
      while (true) {
         std::string epi_isl;
         std::string genome;
         if (!getline(sequences_in, epi_isl)) {
            break;
         }
         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE = 9;
         std::string const tmp = epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE);
         try {
            uint64_t const epi = stoi(tmp);
            if (set.contains(epi)) {
               if (!getline(sequences_in, genome)) {
                  break;
               }
               found_sequences_count++;
               sequences_out << epi_isl << "\n" << genome << "\n";
            } else {
               sequences_in.ignore(LONG_MAX, '\n');
            }
         } catch (const std::invalid_argument& exception) {
            throw silo::PreprocessingException(
               "Failed parsing EPI: " + epi_isl + " (sequence " +
               std::to_string(found_sequences_count) + "): " + exception.what()
            );
         }
      }
   }
   SPDLOG_INFO("Finished reading sequences, found {} sequences", found_sequences_count);
}

void silo::partitionSequences(
   const preprocessing::Partitions& partitions,
   std::istream& meta_in,
   std::istream& sequence_in,
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

   std::unordered_map<uint64_t, std::string> epi_to_chunk;

   {
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
         std::string epi_isl;
         std::string pango_lineage_raw;
         std::string rest;
         if (!getline(meta_in, epi_isl, '\t')) {
            break;
         }
         if (!getline(meta_in, pango_lineage_raw, '\t')) {
            break;
         }
         if (!getline(meta_in, rest, '\n')) {
            break;
         }

         /// Deal with pango_lineage alias:
         std::string const pango_lineage = alias_key.resolvePangoLineageAlias(pango_lineage_raw);

         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL = 8;
         std::string const tmp = epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL);
         uint64_t const epi = stoi(tmp);

         std::string const chunk = pango_to_chunk[pango_lineage];
         *chunk_to_meta_ostream[chunk] << epi_isl << '\t' << pango_lineage << '\t' << rest << '\n';

         // Now saveDatabaseState where the epi will go for the sequence partitioning
         epi_to_chunk[epi] = chunk;
      }
   }

   {
      SPDLOG_INFO("partitioning sequences file to {}", output_prefix);

      std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_seq_ostream;
      for (const std::string& chunk_name : chunk_names) {
         const std::string chunk_sequence_filename =
            std::string(output_prefix).append(chunk_name).append(sequence_file_extension);
         auto out = make_unique<std::ofstream>(chunk_sequence_filename);
         chunk_to_seq_ostream[chunk_name] = std::move(out);
      }
      SPDLOG_DEBUG("Created file streams for {}", output_prefix);

      while (true) {
         std::string epi_isl;
         std::string genome;
         if (!getline(sequence_in, epi_isl)) {
            break;
         }
         if (!getline(sequence_in, genome)) {
            break;
         }
         if (genome.length() != GENOME_LENGTH) {
            throw silo::PreprocessingException(
               "Genome didn't have expected length " + std::to_string(GENOME_LENGTH) + " (was " +
               std::to_string(genome.length()) + ")."
            );
         }
         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE = 9;
         const uint64_t epi = stoi(epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE));

         std::string const chunk = epi_to_chunk.at(epi);
         *chunk_to_seq_ostream[chunk] << epi_isl << '\n' << genome << '\n';
      }
   }
   SPDLOG_INFO("Finished partitioning to {}", output_prefix);
}

struct PartitionChunk {
   uint32_t part;
   uint32_t chunk;
   uint32_t size;
};

void sortChunk(
   std::istream& meta_in,
   std::istream& sequence_in,
   std::ostream& meta_out,
   std::ostream& sequence_out,
   PartitionChunk chunk
) {
   const std::string chunk_str =
      'P' + std::to_string(chunk.part) + '_' + 'C' + std::to_string(chunk.chunk);

   std::unordered_map<uint64_t, time_t> epi_to_date;

   {
      struct MetaLine {
         uint64_t epi;
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
         std::string epi_isl;
         std::string pango_lineage;
         std::string date_str;
         std::string rest;
         if (!getline(meta_in, epi_isl, '\t')) {
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
         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL = 8;
         std::string const tmp = epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL);
         uint64_t const epi = stoi(tmp);

         struct std::tm time_struct {};
         std::istringstream time_stream(date_str);
         time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
         std::time_t const date_time = mktime(&time_struct);

         lines.push_back(MetaLine{epi, pango_lineage, date_time, date_str, rest});

         epi_to_date[epi] = date_time;
      }

      auto sorter = [](const MetaLine& line1, const MetaLine& line2) {
         return line1.date < line2.date;
      };
      std::sort(lines.begin(), lines.end(), sorter);

      meta_out << header << '\n';

      for (const MetaLine& line : lines) {
         meta_out << "EPI_ISL_" << line.epi << '\t' << line.pango << '\t' << line.date_str << '\t'
                  << line.rest << '\n';
      }
   }

   {
      // Now:
      // Read file once, fill all dates, sort dates,
      // calculated target position for every genome
      // Reset gpointer, read file again, putting every genome at the correct position.
      // Write file to ostream

      struct EPIDate {
         uint64_t epi;
         time_t date;
         uint32_t file_pos;
      };
      std::vector<EPIDate> epi_dates;
      epi_dates.reserve(chunk.size);
      uint32_t number_of_epis = 0;
      while (true) {
         std::string epi_isl;
         if (!getline(sequence_in, epi_isl)) {
            break;
         }
         sequence_in.ignore(LONG_MAX, '\n');

         // Add the count to the respective pid

         static constexpr int BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE = 9;
         uint64_t const epi = stoi(epi_isl.substr(BEGIN_OF_NUMBER_IN_EPI_ISL_OF_SEQUENCE_FILE));

         time_t const date = epi_to_date[epi];
         epi_dates.emplace_back(EPIDate{epi, date, number_of_epis++});
      }

      SPDLOG_TRACE("Finished first run for chunk {}", chunk_str);

      auto sorter = [](const EPIDate& date1, const EPIDate& date2) {
         return date1.date < date2.date;
      };
      std::sort(epi_dates.begin(), epi_dates.end(), sorter);

      SPDLOG_TRACE("Sorted first run for partition {}", chunk_str);

      std::vector<uint32_t> file_pos_to_sorted_pos(number_of_epis);
      unsigned number_of_sorted_files = 0;
      for (auto& epi_date : epi_dates) {
         file_pos_to_sorted_pos[epi_date.file_pos] = number_of_sorted_files++;
      }

      SPDLOG_TRACE("Calculated postitions for every sequence {}", chunk_str);

      sequence_in.clear();                  // clear fail and eof bits
      sequence_in.seekg(0, std::ios::beg);  // back to the start!

      SPDLOG_TRACE("Reset file seek, now read second time, sorted {}", chunk_str);

      constexpr uint32_t LINES_PER_SEQUENCE = 2;
      std::vector<std::string> lines_sorted(
         static_cast<uint64_t>(LINES_PER_SEQUENCE * number_of_epis)
      );
      for (auto pos : file_pos_to_sorted_pos) {
         const uint64_t second_line = static_cast<uint64_t>(LINES_PER_SEQUENCE) * pos;
         if (!getline(sequence_in, lines_sorted.at(second_line))) {
            SPDLOG_ERROR("Reached EOF too early.");
            return;
         }
         if (!getline(sequence_in, lines_sorted.at(second_line + 1))) {
            SPDLOG_ERROR("Reached EOF too early.");
            return;
         }
      }

      for (const std::string& line : lines_sorted) {
         sequence_out << line << '\n';
      }
   }
}

void silo::sortChunks(
   const preprocessing::Partitions& partitions,
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
      const auto& file_name = output_prefix + silo::buildChunkName(chunk.part, chunk.chunk);
      silo::InputStreamWrapper const sequence_in(file_name + sequence_file_extension);
      silo::InputStreamWrapper const meta_in(file_name + metadata_file_extension);
      std::ofstream sequence_out(file_name + "_sorted" + sequence_file_extension);
      std::ofstream meta_out(file_name + "_sorted" + metadata_file_extension);
      sortChunk(
         meta_in.getInputStream(), sequence_in.getInputStream(), meta_out, sequence_out, chunk
      );
   });
}
