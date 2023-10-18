#include "silo/common/fasta_reader.h"

#include <climits>
#include <iostream>
#include <string>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/input_stream_wrapper.h"

silo::FastaReader::FastaReader(const std::filesystem::path& in_file_name)
    : in_file(in_file_name) {}

std::optional<std::string> silo::FastaReader::nextKey() {
   std::string key_with_prefix;
   if (!getline(in_file.getInputStream(), key_with_prefix)) {
      return std::nullopt;
   }

   if (key_with_prefix.empty() || key_with_prefix.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + key_with_prefix);
   }

   return key_with_prefix.substr(1);
}

std::optional<std::string> silo::FastaReader::nextSkipGenome() {
   auto key = nextKey();

   in_file.getInputStream().ignore(LONG_MAX, '\n');
   return key;
}

std::optional<std::string> silo::FastaReader::next(std::string& genome_buffer) {
   auto key = nextKey();
   if (!key) {
      return std::nullopt;
   }

   if (!getline(in_file.getInputStream(), genome_buffer)) {
      throw FastaFormatException("Missing genome sequence in line following key: " + *key);
   }

   return key;
}

void silo::FastaReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
