//
// Created by Alexander Taepper on 01.09.22.
//

#include "meta_store.h"

using namespace silo;

/// Deprecated
void silo::analyseMeta(std::istream& in){
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   // vector<string> lineage_vec;
   std::unordered_map<std::string, uint32_t> lineages;
   while (true) {
      int next = in.peek();
      if(next == EOF || next == '\n') break;

      std::string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      /*if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;*/
      if (!getline(in, division, '\n')) break;

      if(pango_lineage.empty()){
         std::cout << "Empty pango-lineage: " << epi_isl << std::endl;
      }

      if (!lineages.contains(pango_lineage)) {
         lineages[pango_lineage] = 1;
      }
      else{
         lineages[pango_lineage]++;
      }
   }

   for (auto &x: lineages)
      std::cout << x.first << ':' << x.second << '\n';

   std::cout << "total partitions: " << lineages.size() << std::endl;
}

static inline void inputSequenceMeta(MetaStore& mdb, uint64_t epi, uint16_t pango_idx, const std::string& date,
                                      const std::string& region, const std::string& country, const std::string& division){
   mdb.epi_to_pid[epi] = pango_idx;
   mdb.pid_to_metacount[pango_idx]++;

   uint32_t sidM = mdb.epi_count++;
   mdb.sidM_to_epi.push_back(epi);
   mdb.epi_to_sidM[epi] = sidM;

   struct std::tm tm{};
   std::istringstream ss(date);
   ss >> std::get_time(&tm, "%Y-%m-%d");
   std::time_t time = mktime(&tm);

   mdb.sidM_to_date.push_back(time);
   mdb.sidM_to_country.push_back(country);
   mdb.sidM_to_region.push_back(region);
}

void silo::processMeta(MetaStore& mdb, std::istream& in){
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


      if(pango_lineage.empty()){
         std::cout << "Empty pango-lineage: " << pango_lineage << " " << epi_isl << std::endl;
      }
      else if(pango_lineage.length()==1 && pango_lineage != "A" && pango_lineage != "B"){
         std::cout << "One-Char pango-lineage:" << epi_isl  << " Lineage:'" << pango_lineage << "'";
         std::cout << "(Keycode=" << (uint) pango_lineage.at(0) << ") may be relevant if it is not printable" << std::endl;
      }

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx;
      if(mdb.pango_to_pid.contains(pango_lineage)){
         pango_idx = mdb.pango_to_pid[pango_lineage];
      }
      else{
         pango_idx = mdb.pid_count++;
         mdb.pid_to_pango.push_back(pango_lineage);
         mdb.pid_to_metacount.push_back(0);
         mdb.pango_to_pid[pango_lineage] = pango_idx;
      }

      inputSequenceMeta(mdb, epi, pango_idx, date, region, country, division);
   }

   if(mdb.epi_count != mdb.epi_to_pid.size()){
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}

void silo::processMeta_ordered(MetaStore& mdb, std::istream& in){
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   while (true) {
      std::string pango_lineage;
      in.ignore(LONG_MAX, '\t');
      if (!getline(in, pango_lineage, '\t')) break;
      in.ignore(LONG_MAX, '\n');

      if(!mdb.pango_to_pid.contains(pango_lineage)){
         mdb.pango_to_pid[pango_lineage] = mdb.pid_count++;
         mdb.pid_to_pango.push_back(pango_lineage);
      }
   }

   // Now sort alphabetically so that we get better compression.
   // -> similar PIDs next to each other in sequence_store -> better run-length compression
   std::sort(mdb.pid_to_pango.begin(), mdb.pid_to_pango.end());

   mdb.pango_to_pid.clear();
   for(uint16_t pid = 0; pid<mdb.pid_count; ++pid){
      auto pango = mdb.pid_to_pango[pid];
      mdb.pango_to_pid[pango] = pid;
   }
   mdb.pid_to_metacount.resize(mdb.pid_count);

   in.clear();                         // clear fail and eof bits
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

      if(pango_lineage.length() < 2) {
         if (pango_lineage.empty()) {
            std::cout << "Empty pango-lineage: " << pango_lineage << " " << epi_isl << std::endl;
         } else if (pango_lineage.length() == 1 && pango_lineage != "A" && pango_lineage != "B") {
            std::cout << "One-Char pango-lineage:" << epi_isl << " Lineage:'" << pango_lineage << "'";
            std::cout << "(Keycode=" << (uint) pango_lineage.at(1) << ")" << std::endl;
         }
      }

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      // Guaranteed to find, due to first-pass above
      uint16_t pango_idx = mdb.pango_to_pid[pango_lineage];

      inputSequenceMeta(mdb, epi, pango_idx, date, region, country, division);
   }

   if(mdb.epi_count != mdb.epi_to_pid.size()){
      std::cout << "ERROR: EPI is represented twice." << std::endl;
   }
}

void silo::meta_info(const MetaStore& mdb, std::ostream& out) {
   out << "Infos by pango:" << std::endl;
   for (unsigned i = 0; i < mdb.pid_count; i++) {
      out << "(pid: " << i << ",\tpango-lin: " <<  mdb.pid_to_pango[i]
         << ",\tcount: " << number_fmt(mdb.pid_to_metacount[i])  << ')' << std::endl;
   }
}

unsigned silo::save_meta(const MetaStore& db, const std::string& db_filename) {
   std::cout << "Writing out meta." << std::endl;

   std::ofstream wf(db_filename, std::ios::out | std::ios::binary);
   if(!wf) {
      std::cerr << "Cannot open ofile: " << db_filename << std::endl;
      return 1;
   }

   {
      boost::archive::binary_oarchive oa(wf);
      // write class instance to archive
      oa << db;
      // archive and stream closed when destructors are called
   }

   return 0;
}

unsigned silo::load_meta(MetaStore& db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, std::ios::binary);
      boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
   return 0;
}