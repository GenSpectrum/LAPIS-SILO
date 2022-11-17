//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo/meta_store.h"

using namespace silo;

static inline void inputSequenceMeta(MetaStore& mdb, uint64_t epi, uint16_t pango_idx, const std::string& date,
                                     const std::string& region, const std::string& country, const std::string& /*TODO division*/) {
   mdb.epi_to_pid[epi] = pango_idx;

   uint32_t sidM = mdb.sequence_count++;
   mdb.sid_to_epi.push_back(epi);
   mdb.epi_to_sid[epi] = sidM;

   struct std::tm tm {};
   std::istringstream ss(date);
   ss >> std::get_time(&tm, "%Y-%m-%d");
   std::time_t time = mktime(&tm);

   mdb.sid_to_date.push_back(time);
   mdb.sid_to_country.push_back(country);
   mdb.sid_to_region.push_back(region);
}

void silo::processMeta(MetaStore& mdb, std::istream& in) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage_raw, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(mdb.alias_key, pango_lineage_raw);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx;
      if (mdb.pango_to_pid.contains(pango_lineage)) {
         pango_idx = mdb.pango_to_pid[pango_lineage];
         ++mdb.pangos[pango_idx].count;
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
   in.ignore(LONG_MAX, '\n');
   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage_raw, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(mdb.alias_key, pango_lineage_raw);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx = mdb.pango_to_pid[pango_lineage];

      inputSequenceMeta(mdb, epi, pango_idx, date, region, country, division);
   }

   assert(mdb.sequence_count == mdb.epi_to_pid.size()); // EPIs should be unique
}

void silo::chunk_info(const MetaStore& mdb, std::ostream& out) {
   out << "Infos by pango:" << std::endl;
   for (unsigned i = 0; i < mdb.chunks.size(); ++i) {
      out << "(chunk: " << i << ",\tprefix: " << mdb.chunks[i].prefix
          << ",\tcount: " << number_fmt(mdb.chunks[i].count)
          << ",\tpango range:" << mdb.pangos[mdb.chunks[i].pids.front()].pango_lineage
          << "-" << mdb.pangos[mdb.chunks[i].pids.back()].pango_lineage
          << ",\tpid vec: ";
      std::copy(mdb.chunks[i].pids.begin(),
                mdb.chunks[i].pids.end(),
                std::ostream_iterator<uint32_t>(std::cout, " "));
      std::cout << ')' << std::endl;
   }
}

unsigned silo::save_meta(const MetaStore& mdb, const std::string& db_filename) {
   std::ofstream wf(db_filename, std::ios::binary);
   if (!wf) {
      std::cerr << "Cannot open ofile: " << db_filename << std::endl;
      return 1;
   }
   boost::archive::binary_oarchive oa(wf);
   oa << mdb;
   return 0;
}

unsigned silo::load_meta(MetaStore& mdb, const std::string& db_filename) {
   std::ifstream ifs(db_filename, std::ios::binary);
   if (!ifs) {
      std::cerr << "Cannot open ifile: " << db_filename << std::endl;
      return 1;
   }
   boost::archive::binary_iarchive ia(ifs);
   ia >> mdb;
   return 0;
}
