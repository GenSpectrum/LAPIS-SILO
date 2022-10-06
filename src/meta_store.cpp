//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo/meta_store.h"

using namespace silo;

static inline void inputSequenceMeta(MetaStore& mdb, uint64_t epi, uint16_t pango_idx, const std::string& date,
                                     const std::string& region, const std::string& country, const std::string& /*TODO division*/) {
   mdb.epi_to_pid[epi] = pango_idx;

   uint32_t sidM = mdb.sequence_count++;
   mdb.sidM_to_epi.push_back(epi);
   mdb.epi_to_sidM[epi] = sidM;

   struct std::tm tm {};
   std::istringstream ss(date);
   ss >> std::get_time(&tm, "%Y-%m-%d");
   std::time_t time = mktime(&tm);

   mdb.sidM_to_date.push_back(time);
   mdb.sidM_to_country.push_back(country);
   mdb.sidM_to_region.push_back(region);
}

void silo::processMeta(MetaStore& mdb, std::istream& in) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   while (true) {
      std::string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      if (pango_lineage.empty()) {
         std::cout << "Empty pango-lineage: " << pango_lineage << " " << epi_isl << std::endl;
      } else if (pango_lineage.length() == 1 && pango_lineage != "A" && pango_lineage != "B") {
         std::cout << "One-Char pango-lineage:" << epi_isl << " Lineage:'" << pango_lineage << "'";
         std::cout << "(Keycode=" << (uint) pango_lineage.at(0) << ") may be relevant if it is not printable" << std::endl;
      }

      /// Deal with pango_lineage alias:
      resolve_alias(mdb.alias_key, pango_lineage);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx;
      if (mdb.pango_to_pid.contains(pango_lineage)) {
         pango_idx = mdb.pango_to_pid[pango_lineage];
         mdb.pangos[pango_idx].count++;
      } else {
         pango_idx = mdb.pid_count++;
         mdb.pangos.push_back({pango_lineage, 1, 0});
         mdb.pango_to_pid[pango_lineage] = pango_idx;
      }

      inputSequenceMeta(mdb, epi, pango_idx, date, region, country, division);
   }

   if (mdb.sequence_count != mdb.epi_to_pid.size()) {
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}

void silo::processMeta_ordered(MetaStore& mdb, std::istream& in) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   while (true) {
      std::string pango_lineage;
      in.ignore(LONG_MAX, '\t');
      if (!getline(in, pango_lineage, '\t')) break;
      in.ignore(LONG_MAX, '\n');

      /// Deal with pango_lineage alias:
      resolve_alias(mdb.alias_key, pango_lineage);

      mdb.sequence_count++;
      if (mdb.pango_to_pid.contains(pango_lineage)) {
         auto pid = mdb.pango_to_pid[pango_lineage];
         mdb.pangos[pid].count++;
      } else {
         mdb.pango_to_pid[pango_lineage] = mdb.pid_count++;
         mdb.pangos.emplace_back(pango_t{pango_lineage, 1, 0});
      }
   }

   // Now sort alphabetically so that we get better compression.
   // -> similar PIDs next to each other in sequence_store -> better run-length compression
   std::sort(mdb.pangos.begin(), mdb.pangos.end(),
             [](const pango_t& lhs, const pango_t& rhs) { return lhs.pango_lineage < rhs.pango_lineage; });

   mdb.pango_to_pid.clear();
   for (uint16_t pid = 0; pid < mdb.pid_count; ++pid) {
      auto pango = mdb.pangos[pid];
      mdb.pango_to_pid[pango.pango_lineage] = pid;
   }

   // Merge pango_lineages, such that partitions are not get very small
   mdb.partitions =
      silo::merge_pangos_to_partitions(mdb.pangos,
                                       mdb.sequence_count / 100, mdb.sequence_count / 200);

   mdb.sequence_count = 0;
   in.clear(); // clear fail and eof bits
   in.seekg(0, std::ios::beg); // back to the start!

   in.ignore(LONG_MAX, '\n');
   while (true) {
      std::string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      resolve_alias(mdb.alias_key, pango_lineage);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      // Guaranteed to find, due to first-pass above
      uint16_t pango_idx = mdb.pango_to_pid[pango_lineage];

      inputSequenceMeta(mdb, epi, pango_idx, date, region, country, division);
   }

   if (mdb.sequence_count != mdb.epi_to_pid.size()) {
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}

std::vector<partition_t> silo::merge_pangos_to_partitions(std::vector<pango_t>& pangos,
                                                          unsigned target_size, unsigned min_size) {
   std::list<partition_t> partitions;
   for (uint32_t pid = 0; pid < pangos.size(); pid++) {
      std::vector<uint32_t> v;
      v.push_back(pid);
      partition_t tmp = {pangos[pid].pango_lineage, pangos[pid].count, v};
      partitions.emplace_back(tmp);
   }
   uint32_t max_len = std::max_element(pangos.begin(), pangos.end(),
                                       [](const pango_t& lhs, const pango_t& rhs) {
                                          return lhs.pango_lineage.size() < rhs.pango_lineage.size();
                                       })
                         ->pango_lineage.size();
   for (uint32_t len = max_len; len > 0; len--) {
      for (auto it = partitions.begin(); it != partitions.end() && std::next(it) != partitions.end();) {
         auto&& [pango1, pango2] = std::tie(*it, *std::next(it));
         std::string pref = common_pango_prefix(pango1.prefix, pango2.prefix);
         if (pref.size() == len &&
             (pango1.count < min_size || pango2.count < min_size ||
              (pango1.count < target_size && pango2.count < target_size))) {
            pango2.prefix = pref;
            pango2.count += pango1.count;
            pango2.pids.insert(pango2.pids.end(),
                               pango1.pids.begin(),
                               pango1.pids.end());
            it = partitions.erase(it);

         } else {
            ++it;
         }
      }
   }

   uint32_t part_id = 0;
   for (auto& partition : partitions) {
      std::sort(partition.pids.begin(), partition.pids.end());
      for (const auto& pid : partition.pids) {
         pangos[pid].partition = part_id;
      }
      ++part_id;
   }

   std::vector<partition_t> ret;
   std::copy(
      std::begin(partitions),
      std::end(partitions),
      std::back_inserter(ret));
   return ret;
}

void silo::pango_info(const MetaStore& mdb, std::ostream& out) {
   out << "Infos by pango:" << std::endl;
   for (unsigned i = 0; i < mdb.pid_count; i++) {
      out << "(pid: " << i << ",\tpango-lin: " << mdb.pangos[i].pango_lineage
          << ",\tcount: " << number_fmt(mdb.pangos[i].count)
          << ",\tpartition: " << number_fmt(mdb.pangos[i].partition) << ')' << std::endl;
   }
}

void silo::partition_info(const MetaStore& mdb, std::ostream& out) {
   out << "Infos by pango:" << std::endl;
   for (unsigned i = 0; i < mdb.partitions.size(); i++) {
      out << "(partition: " << i << ",\tprefix: " << mdb.partitions[i].prefix
          << ",\tcount: " << number_fmt(mdb.partitions[i].count)
          << ",\tpango range:" << mdb.pangos[mdb.partitions[i].pids.front()].pango_lineage
          << "-" << mdb.pangos[mdb.partitions[i].pids.back()].pango_lineage
          << ",\tpid vec: ";
      std::copy(mdb.partitions[i].pids.begin(),
                mdb.partitions[i].pids.end(),
                std::ostream_iterator<uint32_t>(std::cout, " "));
      std::cout << ')' << std::endl;
   }
}

unsigned silo::save_meta(const MetaStore& mdb, const std::string& db_filename) {
   std::cout << "Writing out meta." << std::endl;

   std::ofstream wf(db_filename, std::ios::binary);
   if (!wf) {
      std::cerr << "Cannot open ofile: " << db_filename << std::endl;
      return 1;
   }

   {
      boost::archive::binary_oarchive oa(wf);
      // write class instance to archive
      oa << mdb;
      // archive and stream closed when destructors are called
   }
   return 0;
}

unsigned silo::load_meta(MetaStore& mdb, const std::string& db_filename) {
   // create and open an archive for input
   std::ifstream ifs(db_filename, std::ios::binary);
   {
      boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> mdb;
      // archive and stream closed when destructors are called
   }
   return 0;
}
