//
// Created by Alexander Taepper on 16.11.22.
//

#include "silo/prepare_dataset.h"

#include <silo/common/istream_wrapper.h>
#include <silo/database.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for_each.h>
#include <syncstream>
#include <unordered_set>

void silo::prune_meta(std::istream& meta_in, std::istream& sequences_in, std::ostream& meta_out) {
   std::unordered_set<uint64_t> set;
   uint32_t found_seq = 0;
   uint32_t found_meta = 0;
   {
      while (true) {
         std::string epi_isl, genome;
         if (!getline(sequences_in, epi_isl))
            break;
         sequences_in.ignore(LONG_MAX, '\n');

         std::string tmp = epi_isl.substr(9);
         try {
            uint64_t epi = stoi(tmp);
            set.insert(epi);
            found_seq++;
         } catch (const std::invalid_argument& ia) {
            std::cerr << "Failed parsing EPI: " << epi_isl << " Found seq: " << found_seq
                      << std::endl;
            std::cerr << ia.what() << std::endl;
            return;
         }
      }
   }
   std::cout << "Finished seq_reading (" << found_seq << ")" << std::endl;
   {
      std::string header;
      if (!getline(meta_in, header, '\n')) {
         std::cerr << "Meta-file is emtpy. At least Header is expected." << std::endl;
         return;
      }
      meta_out << header << "\n";

      while (true) {
         std::string epi_isl, rest;
         if (!getline(meta_in, epi_isl, '\t'))
            break;

         std::string tmp = epi_isl.substr(8);
         try {
            uint64_t epi = stoi(tmp);
            if (set.contains(epi)) {
               if (!getline(meta_in, rest))
                  break;
               found_meta++;
               meta_out << epi_isl << "\t" << rest << "\n";
            } else {
               meta_in.ignore(LONG_MAX, '\n');
            }
         } catch (const std::invalid_argument& ia) {
            std::cerr << "Failed parsing EPI: " << epi_isl << " Found meta: " << found_meta
                      << std::endl;
            std::cerr << ia.what() << std::endl;
            return;
         }
      }
   }
   std::cout << "Found Seq: " << found_seq << "\nFound Meta: " << found_meta << std::endl;
}

void silo::prune_sequences(
   std::istream& meta_in,
   std::istream& sequences_in,
   std::ostream& sequences_out
) {
   std::unordered_set<uint64_t> set;
   uint32_t found_meta = 0;
   {
      std::string header;
      if (!getline(meta_in, header, '\n')) {
         std::cerr << "Meta-file is emtpy. At least Header is expected." << std::endl;
         return;
      }

      while (true) {
         std::string epi_isl, rest;
         if (!getline(meta_in, epi_isl, '\t'))
            break;
         meta_in.ignore(LONG_MAX, '\n');

         std::string tmp = epi_isl.substr(8);
         try {
            uint64_t epi = stoi(tmp);
            set.insert(epi);
            found_meta++;
         } catch (const std::invalid_argument& ia) {
            std::cerr << "Failed parsing EPI: " << epi_isl << " Found meta: " << found_meta
                      << std::endl;
            std::cerr << ia.what() << std::endl;
            return;
         }
      }
   }
   std::cout << "Finished meta_reading (" << found_meta << ")" << std::endl;
   uint32_t found_seq = 0;
   {
      while (true) {
         std::string epi_isl, genome;
         if (!getline(sequences_in, epi_isl))
            break;

         std::string tmp = epi_isl.substr(9);
         try {
            uint64_t epi = stoi(tmp);
            if (set.contains(epi)) {
               if (!getline(sequences_in, genome))
                  break;
               found_seq++;
               sequences_out << epi_isl << "\n" << genome << "\n";
            } else {
               sequences_in.ignore(LONG_MAX, '\n');
            }
         } catch (const std::invalid_argument& ia) {
            std::cerr << "Failed parsing EPI: " << epi_isl << " Found seq: " << found_seq
                      << std::endl;
            std::cerr << ia.what() << std::endl;
            return;
         }
      }
   }
   std::cout << "Found Seq: " << found_seq << "\nFound Meta: " << found_meta << std::endl;
}

silo::pango_descriptor_t silo::build_pango_defs(
   const std::unordered_map<std::string, std::string>& alias_key,
   std::istream& meta_in
) {
   silo::pango_descriptor_t pango_defs;
   // Ignore header line.
   meta_in.ignore(LONG_MAX, '\n');

   uint32_t pid_count = 0;

   std::unordered_map<std::string, uint32_t> pango_to_id;

   while (true) {
      std::string epi_isl, pango_lineage_raw;
      if (!getline(meta_in, epi_isl, '\t'))
         break;
      if (!getline(meta_in, pango_lineage_raw, '\t'))
         break;
      meta_in.ignore(LONG_MAX, '\n');

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(alias_key, pango_lineage_raw);

      if (pango_to_id.contains(pango_lineage)) {
         auto pid = pango_to_id[pango_lineage];
         ++pango_defs.pangos[pid].count;
      } else {
         pango_to_id[pango_lineage] = pid_count++;
         pango_defs.pangos.emplace_back(pango_t{pango_lineage, 1});
      }
   }

   // Now sort alphabetically so that we get better compression.
   // -> similar PIDs next to each other in sequence_store -> better run-length compression
   std::sort(
      pango_defs.pangos.begin(), pango_defs.pangos.end(),
      [](const pango_t& lhs, const pango_t& rhs) { return lhs.pango_lineage < rhs.pango_lineage; }
   );
   return pango_defs;
}

static std::string common_pango_prefix(const std::string& s1, const std::string& s2) {
   std::string prefix;
   // Buffer until it reaches another .
   std::string buffer;
   unsigned min_len = std::min(s1.length(), s2.length());
   for (unsigned i = 0; i < min_len; i++) {
      if (s1[i] != s2[i])
         return prefix;
      else if (s1[i] == '.') {
         prefix += buffer + '.';
         buffer = "";
      } else {
         buffer += s1[i];
      }
   }
   return prefix + buffer;
}

using pango_t = silo::pango_t;
/// Takes pango_lineages as initial chunk and merges them, trying to merge more closely related ones
/// first Will merge 2 chunks if on is smaller than min_size or both are smaller than target_size
/// Updates pango_lineages to contain the chunk each pango_lineage is contained in and returns
/// vector of chunks
std::vector<silo::chunk_t> merge_pangos_to_chunks(
   std::vector<pango_t>& pangos,
   unsigned target_size,
   unsigned min_size
) {
   // Initialize chunks such that every chunk is just a pango_lineage
   std::list<silo::chunk_t> chunks;
   uint32_t running_total = 0;
   for (auto& pango : pangos) {
      std::vector<std::string> v;
      v.push_back(pango.pango_lineage);
      silo::chunk_t tmp = {pango.pango_lineage, pango.count, running_total, v};
      running_total += pango.count;
      chunks.emplace_back(tmp);
   }
   // We want to prioritise merges more closely related chunks.
   // Therefore, we first merge the chunks, with longer matching prefixes.
   // Precalculate the longest a prefix can be (which is the max length of lineages)
   uint32_t max_len =
      std::max_element(pangos.begin(), pangos.end(), [](const pango_t& lhs, const pango_t& rhs) {
         return lhs.pango_lineage.size() < rhs.pango_lineage.size();
      })->pango_lineage.size();
   for (uint32_t len = max_len; len > 0; len--) {
      for (auto it = chunks.begin(); it != chunks.end() && std::next(it) != chunks.end();) {
         auto&& [pango1, pango2] = std::tie(*it, *std::next(it));
         std::string common_prefix = common_pango_prefix(pango1.prefix, pango2.prefix);
         // We only look at possible merges with a common_prefix length of #len
         bool one_chunk_is_very_small = pango1.count < min_size || pango2.count < min_size;
         bool both_chunks_still_want_to_grow =
            pango1.count < target_size && pango2.count < target_size;
         if (common_prefix.size() == len && (one_chunk_is_very_small || both_chunks_still_want_to_grow)) {
            pango2.prefix = common_prefix;
            pango2.count += pango1.count;
            pango2.pangos.insert(pango2.pangos.end(), pango1.pangos.begin(), pango1.pangos.end());

            // We merged pango1 into pango2 -> Now delete pango1
            // Do not need to increment, because erase will make it automatically point to next
            // element
            it = chunks.erase(it);
         } else {
            ++it;
         }
      }
   }

   std::vector<silo::chunk_t> ret;
   std::copy(std::begin(chunks), std::end(chunks), std::back_inserter(ret));
   return ret;
}

silo::partitioning_descriptor_t silo::build_partitioning_descriptor(
   silo::pango_descriptor_t pango_defs,
   architecture_type arch
) {
   uint32_t total_count = 0;
   for (auto& x : pango_defs.pangos) {
      total_count += x.count;
   }

   silo::partitioning_descriptor_t descriptor;

   switch (arch) {
      case architecture_type::max_partitions:
         for (auto& chunk :
              merge_pangos_to_chunks(pango_defs.pangos, total_count / 100, total_count / 200)) {
            descriptor.partitions.push_back(silo::partition_t{});
            descriptor.partitions.back().name = "full";
            descriptor.partitions.back().chunks.push_back(chunk);
            descriptor.partitions.back().count = chunk.count;
         }
         return descriptor;
      case architecture_type::single_partition:
         descriptor.partitions.push_back(silo::partition_t{});

         descriptor.partitions[0].name = "full";

         // Merge pango_lineages, such that chunks are not get very small
         descriptor.partitions[0].chunks =
            merge_pangos_to_chunks(pango_defs.pangos, total_count / 100, total_count / 200);

         descriptor.partitions[0].count = total_count;
         return descriptor;
      case architecture_type::single_single:

         descriptor.partitions.push_back(silo::partition_t{});
         descriptor.partitions[0].name = "full_full";

         // Merge pango_lineages, such that chunks are not get very small
         descriptor.partitions[0].chunks.push_back(silo::chunk_t{
            "", total_count, 0, std::vector<std::string>()});
         for (auto& pango : pango_defs.pangos) {
            descriptor.partitions[0].chunks.back().pangos.push_back(pango.pango_lineage);
         }

         descriptor.partitions[0].count = total_count;
         return descriptor;
      case hybrid:
         break;
   }
   throw std::runtime_error("Arch not yet implemented.");
}

silo::partitioning_descriptor_t silo::load_partitioning_descriptor(std::istream& in) {
   silo::partitioning_descriptor_t descriptor = {std::vector<partition_t>()};
   std::string type, name, size_str, count_str, offset_str;
   uint32_t count, offset;
   while (in && !in.eof()) {
      if (!getline(in, type, '\t'))
         break;

      if (type.size() != 1) {
         throw std::runtime_error("load_partitioning_descriptor format exception");
      } else if (type.at(0) == 'P') {
         if (!getline(in, name, '\t'))
            break;
         if (!getline(in, size_str, '\t'))
            break;
         if (!getline(in, count_str, '\n'))
            break;
         // size = atoi(size_str.c_str()); unused, only meta information
         count = atoi(count_str.c_str());

         silo::partition_t part{name, count, std::vector<silo::chunk_t>()};
         descriptor.partitions.push_back(part);
      } else if (type.at(0) == 'C') {
         if (!getline(in, name, '\t'))
            break;
         if (!getline(in, size_str, '\t'))
            break;
         if (!getline(in, count_str, '\t'))
            break;
         if (!getline(in, offset_str, '\n'))
            break;
         // size = atoi(size_str.c_str()); unused, only meta information
         count = atoi(count_str.c_str());
         offset = atoi(offset_str.c_str());
         silo::chunk_t chunk{name, count, offset, std::vector<std::string>()};
         descriptor.partitions.back().chunks.push_back(chunk);
      } else if (type.at(0) == 'L') {
         if (!getline(in, name, '\n'))
            break;
         descriptor.partitions.back().chunks.back().pangos.push_back(name);
      } else {
         throw std::runtime_error("load_partitioning_descriptor format exception");
      }
   }
   return descriptor;
}

void silo::partition_sequences(
   const partitioning_descriptor_t& pd,
   std::istream& meta_in,
   std::istream& sequence_in,
   const std::string& output_prefix,
   const std::unordered_map<std::string, std::string>& alias_key,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
) {
   std::unordered_map<std::string, std::string> pango_to_chunk;
   std::vector<std::string> chunk_strs;
   for (unsigned i = 0, limit = pd.partitions.size(); i < limit; ++i) {
      const auto& part = pd.partitions[i];
      for (unsigned j = 0, limit2 = part.chunks.size(); j < limit2; ++j) {
         auto& chunk = part.chunks[j];
         chunk_strs.push_back(silo::chunk_string(i, j));
         for (auto& pango : chunk.pangos) {
            pango_to_chunk[pango] = chunk_strs.back();
         }
      }
   }

   std::unordered_map<uint64_t, std::string> epi_to_chunk;

   {
      std::cout << "Now partitioning metafile to " << output_prefix << std::endl;

      std::string header;
      if (!getline(meta_in, header, '\n')) {
         std::cerr << "No header file in meta input." << std::endl;
         return;
      }

      std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_meta_ostream;
      for (const std::string& chunk_s : chunk_strs) {
         auto out = make_unique<std::ofstream>(output_prefix + chunk_s + metadata_file_extension);
         chunk_to_meta_ostream[chunk_s] = std::move(out);
         *chunk_to_meta_ostream[chunk_s] << header << '\n';
      }

      while (true) {
         std::string epi_isl, pango_lineage_raw, rest;
         if (!getline(meta_in, epi_isl, '\t'))
            break;
         if (!getline(meta_in, pango_lineage_raw, '\t'))
            break;
         if (!getline(meta_in, rest, '\n'))
            break;

         /// Deal with pango_lineage alias:
         std::string pango_lineage = resolve_alias(alias_key, pango_lineage_raw);

         std::string tmp = epi_isl.substr(8);
         uint64_t epi = stoi(tmp);

         std::string chunk = pango_to_chunk[pango_lineage];
         *chunk_to_meta_ostream[chunk] << epi_isl << '\t' << pango_lineage << '\t' << rest << '\n';

         // Now save where the epi will go for the sequence partitioning
         epi_to_chunk[epi] = chunk;
      }
   }

   {
      std::cout << "Now partitioning fasta file to " << output_prefix << std::endl;
      std::unordered_map<std::string, std::unique_ptr<std::ostream>> chunk_to_seq_ostream;
      for (const std::string& chunk_s : chunk_strs) {
         auto out = make_unique<std::ofstream>(output_prefix + chunk_s + sequence_file_extension);
         chunk_to_seq_ostream[chunk_s] = std::move(out);
      }
      std::cout << "Created file streams for  " << output_prefix << std::endl;
      while (true) {
         std::string epi_isl, genome;
         if (!getline(sequence_in, epi_isl))
            break;
         if (!getline(sequence_in, genome))
            break;
         if (genome.length() != genomeLength) {
            std::cerr << "length mismatch!" << std::endl;
            return;
         }
         const uint64_t epi = stoi(epi_isl.substr(9));

         std::string chunk = epi_to_chunk.at(epi);
         *chunk_to_seq_ostream[chunk] << epi_isl << '\n' << genome << '\n';
      }
   }
   std::cout << "Finished partitioning to " << output_prefix << std::endl;
}

struct part_chunk {
   uint32_t part;
   uint32_t chunk;
   uint32_t size;

   part_chunk(uint32_t part, uint32_t chunk, uint32_t size)
       : part(part),
         chunk(chunk),
         size(size) {}
};

void sort_chunk(
   std::istream& meta_in,
   std::istream& sequence_in,
   std::ostream& meta_out,
   std::ostream& sequence_out,
   part_chunk chunk_d
) {
   const std::string chunk_str =
      'P' + std::to_string(chunk_d.part) + '_' + 'C' + std::to_string(chunk_d.chunk);

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
      lines.reserve(chunk_d.size);

      // Ignore Header
      std::string header;
      if (!getline(meta_in, header, '\n')) {
         std::cerr << "No header in metadata file. Abort." << std::endl;
         return;
      }
      while (true) {
         std::string epi_isl, pango_lineage, date_str, rest;
         if (!getline(meta_in, epi_isl, '\t'))
            break;
         if (!getline(meta_in, pango_lineage, '\t'))
            break;
         if (!getline(meta_in, date_str, '\t'))
            break;
         if (!getline(meta_in, rest, '\n'))
            break;

         std::string tmp = epi_isl.substr(8);
         uint64_t epi = stoi(tmp);

         struct std::tm tm {};
         std::istringstream ss(date_str);
         ss >> std::get_time(&tm, "%Y-%m-%d");
         std::time_t date_time = mktime(&tm);

         lines.push_back(MetaLine{epi, pango_lineage, date_time, date_str, rest});

         epi_to_date[epi] = date_time;
      }

      auto sorter = [](const MetaLine& s1, const MetaLine& s2) { return s1.date < s2.date; };
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
      std::vector<EPIDate> firstRun;
      firstRun.reserve(chunk_d.size);

      uint32_t count = 0;
      while (true) {
         std::string epi_isl;
         if (!getline(sequence_in, epi_isl))
            break;
         sequence_in.ignore(LONG_MAX, '\n');

         // Add the count to the respective pid
         uint64_t epi = stoi(epi_isl.substr(9));

         time_t date = epi_to_date[epi];
         firstRun.emplace_back(EPIDate{epi, date, count++});
      }

      std::osyncstream(std::cout) << "Finished first run for chunk: " << chunk_str << std::endl;

      auto sorter = [](const EPIDate& s1, const EPIDate& s2) { return s1.date < s2.date; };
      std::sort(firstRun.begin(), firstRun.end(), sorter);

      std::osyncstream(std::cout) << "Sorted first run for partition: " << chunk_str << std::endl;

      std::vector<uint32_t> file_pos_to_sorted_pos(count);
      unsigned count2 = 0;
      for (auto& x : firstRun) {
         file_pos_to_sorted_pos[x.file_pos] = count2++;
      }

      assert(count == count2);

      std::osyncstream(std::cout) << "Calculated postitions for every sequence: " << chunk_str
                                  << std::endl;

      sequence_in.clear();                  // clear fail and eof bits
      sequence_in.seekg(0, std::ios::beg);  // back to the start!

      std::osyncstream(std::cout) << "Reset file seek, now read second time, sorted: " << chunk_str
                                  << std::endl;

      std::vector<std::string> lines_sorted(2 * count);
      for (auto pos : file_pos_to_sorted_pos) {
         std::string epi_isl, genome;
         if (!getline(sequence_in, lines_sorted[2 * pos])) {
            std::cerr << "Reached EOF too early." << std::endl;
            return;
         }
         if (!getline(sequence_in, lines_sorted[2 * pos + 1])) {
            std::cerr << "Reached EOF too early." << std::endl;
            return;
         }
      }

      for (const std::string& line : lines_sorted) {
         sequence_out << line << '\n';
      }
   }
}

void silo::sort_chunks(
   const partitioning_descriptor_t& pd,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
) {
   std::vector<part_chunk> all_chunks;
   for (uint32_t part_id = 0, limit = pd.partitions.size(); part_id < limit; ++part_id) {
      const auto& part = pd.partitions[part_id];
      for (uint32_t chunk_id = 0, limit2 = part.chunks.size(); chunk_id < limit2; ++chunk_id) {
         const auto& chunk = part.chunks[chunk_id];
         all_chunks.emplace_back(part_id, chunk_id, chunk.count);
      }
   }

   tbb::parallel_for_each(all_chunks.begin(), all_chunks.end(), [&](const part_chunk& x) {
      const std::string& file_name = output_prefix + silo::chunk_string(x.part, x.chunk);
      silo::istream_wrapper sequence_in(file_name + sequence_file_extension);
      silo::istream_wrapper meta_in(file_name + metadata_file_extension);
      std::ofstream sequence_out(file_name + "_sorted" + sequence_file_extension);
      std::ofstream meta_out(file_name + "_sorted" + metadata_file_extension);
      sort_chunk(meta_in.get_is(), sequence_in.get_is(), meta_out, sequence_out, x);
   });
}
