//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo.h"
#include <boost/serialization/unordered_map.hpp>

#include <boost/serialization/vector.hpp>


struct MetaStore{
   friend class boost::serialization::access;
   template<class Archive>
   void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
   {
      ar & pid_count;
      ar & epi_count;

      // vector
      ar & pid_to_pango;
      ar & pid_to_offset;

      // unordered_map
      ar & pango_to_pid;
      ar & epi_to_pid;
   }

   uint64_t epi_count = 0;
   uint16_t pid_count = 0;

   // Maps the epis to the ID, which is assigned to the pango lineage (pid)
   // pids are starting at 0 and are dense, so that we can save the respective data in vectors.
   unordered_map<uint64_t, uint16_t> epi_to_pid;

   vector<string> pid_to_pango;
   unordered_map<string, uint16_t> pango_to_pid;

   // pid to offsets
   vector<uint32_t> pid_to_offset;
};


// Meta-Data is input
void analyseMeta(istream& in){
   in.ignore(LONG_MAX, '\n');

   // vector<string> lineage_vec;
   std::unordered_map<string, uint32_t> lineages;
   while (true) {
      int next = in.peek();
      if(next == EOF || next == '\n') break;

      string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      /*if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;*/
      if (!getline(in, division, '\n')) break;

      if(pango_lineage.empty()){
         cout << "Empty pango-lineage: " << epi_isl << endl;
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

   cout << "total partitions: " << lineages.size() << endl;
}


// Meta-Data is input
void processMeta(MetaStore& mdb, istream& in){
   in.ignore(LONG_MAX, '\n');

   while (true) {
      string epi_isl, pango_lineage, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage, '\t')) break;
      /*if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;*/
      if (!getline(in, division, '\n')) break;


      if(pango_lineage.empty()){
         cout << "Empty pango-lineage: " << pango_lineage << " " << epi_isl << endl;
      }
      else if(pango_lineage.length()==1 && pango_lineage != "A" && pango_lineage != "B"){
         cout << "One-Char pango-lineage:" << epi_isl  << " Lineage:'" << pango_lineage << "'";
         cout << "(Keycode=" << (uint) pango_lineage.at(1) << ")" << endl;
      }

      string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx;
      if(mdb.pango_to_pid.contains(pango_lineage)){
         pango_idx = mdb.pango_to_pid[pango_lineage];
      }
      else{
         pango_idx = mdb.pid_count++;
         mdb.pid_to_pango.push_back(pango_lineage);
         mdb.pango_to_pid[pango_lineage] = pango_idx;
      }
      // Note that epi may not be contained twice. Can later be checked by epi_count == epi_to_pid.size()
      mdb.epi_count++;
      mdb.epi_to_pid[epi] = pango_idx;
   }

   if(mdb.epi_count != mdb.epi_to_pid.size()){
      cout << "ERROR: EPI is represented twice." << endl;
   }
}

void calc_partition_offsets(MetaStore& mdb, istream& in){
   cout << "Now calculating partition offsets" << endl;

   // Clear the vector and resize
   mdb.pid_to_offset.clear();
   mdb.pid_to_offset.resize(mdb.pid_count + 1);

   while (true) {
      string epi_isl;
      if (!getline(in, epi_isl)) break;
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Add the count to the respective pid
      uint64_t epi = stoi(epi_isl.substr(9));
      if(mdb.epi_to_pid.contains(epi)) {
         uint16_t pid = mdb.epi_to_pid[epi];
         mdb.pid_to_offset[pid]++;
      }
      else{
         uint16_t pid = mdb.pid_count;
         mdb.pid_to_offset[pid]++;
      }
   }

   // Escalate offsets from start to finish
   uint32_t cumulative_offset = 0;
   for(uint32_t& offset : mdb.pid_to_offset){
      auto tmp = offset;
      offset = cumulative_offset;
      cumulative_offset += tmp;
   }

   // cumulative_offset should be equal to sequence count now

   cout << "Finished calculating partition offsets." << endl;
}

void meta_info(const MetaStore& mdb, ostream& out){
   out << "pango_to_pid:" << endl;
   for (auto &x: mdb.pango_to_pid)
      out << x.first << ':' << x.second << endl;
}

static unsigned save_meta(const MetaStore& db, const std::string& db_filename) {
   std::cout << "Writing out meta." << std::endl;

   ofstream wf(db_filename, ios::out | ios::binary);
   if(!wf) {
      cerr << "Cannot open ofile: " << db_filename << endl;
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

static unsigned load_meta(MetaStore& db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, ios::binary);
      boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
   return 0;
}