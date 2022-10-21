//
// Created by Alexander Taepper on 30.09.22.
//
#include "meta_store_test.cpp"
#include "query_test.cpp"
#include "silo/silo.h"

int main(int argc, char* argv[]) {
   if (argc < 2) {
      std::cerr << "No test specified" << std::endl;
      return -1;
   }
   std::string arg(argv[1]);
   if (arg == "resolve_alias") {
      resolve_alias_test();
   } else if (arg == "pango_util") {
   } else {
      std::cerr << "Unknown Test. " << arg << std::endl;
      return -1;
   }
}

int limit_meta_to_seqs() {
   std::unordered_set<uint64_t> set;
   uint32_t found_seq = 0;
   uint32_t found_meta = 0;
   {
      std::ifstream in("../Data/aligned.50k.fasta");
      while (true) {
         std::string epi_isl, genome;
         if (!getline(in, epi_isl)) break;
         in.ignore(LONG_MAX, '\n');

         std::string tmp = epi_isl.substr(9);
         try {
            uint64_t epi = stoi(tmp);
            set.insert(epi);
            found_seq++;
         } catch (const std::invalid_argument& ia) {
            std::cout << ia.what() << std::endl
                      << tmp << std::endl;
            return 5;
         }
      }
   }
   std::cout << "Finished seq_reading (" << found_seq << ")" << std::endl;
   {
      std::ifstream in("../Data/metadata.tsv");
      std::ofstream out("../Data/metadata.50k.tsv", std::ios::out);
      std::string header;
      if (!getline(in, header, '\n')) { return 7; }
      out << header << "\n";

      while (true) {
         std::string epi_isl, rest;
         if (!getline(in, epi_isl, '\t')) break;
         if (!getline(in, rest, '\n')) break;

         std::string tmp = epi_isl.substr(8);
         uint64_t epi = stoi(tmp);
         if (set.contains(epi)) {
            found_meta++;
            out << epi_isl << "\t"
                << rest << "\n";
         }
      }
   }
   std::cout << "Found Seq: " << found_seq << "\nFount Meta: " << found_meta << std::endl;
   return 0;
}
