#include "silo/common/fasta_reader.h"

#include <climits>
#include <iostream>
#include <string>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/input_stream_wrapper.h"

silo::FastaReader::FastaReader(const std::filesystem::path& in_file_name)
    : in_file(in_file_name) {}

bool silo::FastaReader::populateKey(std::string& key) {
   std::string key_with_prefix;
   if (!getline(in_file.getInputStream(), key_with_prefix)) {
      return false;
   }

   if (key_with_prefix.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + key_with_prefix);
   }

   key = key_with_prefix.substr(1);
   return true;
}

bool silo::FastaReader::nextKey(std::string& key) {
   auto key_was_read = populateKey(key);

   in_file.getInputStream().ignore(LONG_MAX, '\n');
   return key_was_read;
}

bool silo::FastaReader::next(std::string& key, std::string& genome) {
   auto key_was_read = populateKey(key);
   if (!key_was_read) {
      return false;
   }

   if (!getline(in_file.getInputStream(), genome)) {
      throw FastaFormatException("Missing genome sequence in line following key: " + key);
   }

   return true;
}

void silo::FastaReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
