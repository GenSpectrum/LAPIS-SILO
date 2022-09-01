//
// Created by Alexander Taepper on 01.09.22.
//

#include "silo.h"

struct MetaStore{
   friend class boost::serialization::access;
   template<class Archive>
   void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
   {
      // TODO
   }

   vector<uint32_t> partition_to_offset;
   unsigned sequenceCount = 0;

   [[nodiscard]] size_t computeSize() const {
      // TODO
      return 0;
   }
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
void processMeta(istream& in, vector<string>& pid_to_pango, unordered_map<string, uint16_t>& pango_to_pid,
                 unordered_map<uint64_t, uint16_t>& epi_to_pid){
   in.ignore(LONG_MAX, '\n');

   uint16_t next_idx = 0;
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
         cout << "Empty pango-lineage: " << pango_lineage << " " << epi_isl << endl;
      }
      else if(pango_lineage.length()==1 && pango_lineage != "A" && pango_lineage != "B"){
         cout << "One-Char pango-lineage:" << epi_isl  << " Lineage:'" << pango_lineage << "'";
         cout << "(Keycode=" << (uint) pango_lineage.at(1) << ")" << endl;
      }

      string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);
      uint16_t pango_idx;
      if(pango_to_pid.contains(pango_lineage)){
         pango_idx = pango_to_pid[pango_lineage];
      }
      else{
         pango_idx = next_idx++;
         pid_to_pango.push_back(pango_lineage);
         pango_to_pid[pango_lineage] = pango_idx;
      }
      epi_to_pid[epi] = pango_idx;
   }

   cout << "pango_to_pid:" << endl;
   for (auto &x: pango_to_pid)
      std::cout << x.first << ':' << x.second << '\n';

   cout << "Built Meta-indices." << endl;
}