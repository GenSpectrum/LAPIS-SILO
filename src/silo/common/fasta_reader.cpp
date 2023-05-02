#include "silo/common/fasta_reader.h"

silo::FastaReader::FastaReader(std::string in_file_name)
    : in_file(in_file_name) {}

bool silo::FastaReader::nextKey(std::string& key) {
   std::string key_with_prefix;
   if (!getline(in_file.getInputStream(), key_with_prefix)) {
      return false;
   }

   if (key_with_prefix.at(0) != '>') {
      return false;
   }

   key = key_with_prefix.substr(1);

   in_file.getInputStream().ignore(LONG_MAX, '\n');
   return true;
}

bool silo::FastaReader::next(std::string& key, std::string& genome) {
   std::string key_with_prefix;
   if (!getline(in_file.getInputStream(), key_with_prefix)) {
      return false;
   }

   if (key_with_prefix.at(0) != '>') {
      return false;
   }

   if (!getline(in_file.getInputStream(), genome)) {
      return false;
   }

   key = key_with_prefix.substr(1);
   return true;
}

void silo::FastaReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
