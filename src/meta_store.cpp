//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo/meta_store.h"

using namespace silo;

static inline void inputSequenceMeta(MetaStore& mdb, uint64_t epi, const std::string& pango_lineage, const std::string& date,
                                     const std::string& region, const std::string& country, const std::string& /*TODO division*/) {
   uint32_t sidM = mdb.sequence_count++;
   mdb.sid_to_epi.push_back(epi);
   mdb.epi_to_sid[epi] = sidM;
   mdb.sid_to_lineage.push_back(pango_lineage);

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

      inputSequenceMeta(mdb, epi, pango_lineage, date, region, country, division);
   }

   if (mdb.sequence_count != mdb.epi_to_pid.size()) {
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}

void silo::chunk_info(const MetaStore& mdb, std::ostream& out) {
   out << "Infos by pango:" << std::endl;
   for (unsigned i = 0; i < mdb.chunks.size(); ++i) {
      out << "(chunk: " << i << ",\tprefix: " << mdb.chunks[i].prefix
          << ",\tcount: " << number_fmt(mdb.chunks[i].count)
          << ",\tpango vec: ";
      std::copy(mdb.chunks[i].pangos.begin(),
                mdb.chunks[i].pangos.end(),
                std::ostream_iterator<std::string>(std::cout, " "));
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
