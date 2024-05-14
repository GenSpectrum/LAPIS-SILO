#include "silo/common/fasta_reader.h"

#include <climits>
#include <iostream>
#include <string>

#include "silo/common/fasta_format_exception.h"
#include "silo/common/input_stream_wrapper.h"

silo::FastaReader::FastaReader(const std::filesystem::path& in_file_name)
    : in_file(in_file_name) {}

std::optional<silo::FastaReader::SequenceIdentifier> silo::FastaReader::nextIdentifier() {
   std::string data;
   if (!getline(in_file.getInputStream(), data)) {
      return std::nullopt;
   }

   if (data.empty() || data.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + data);
   }

   silo::FastaReader::SequenceIdentifier identifier(data.substr(1));

   if (identifier.key.empty()) {
      throw FastaFormatException("Fasta description not valid, missing id: " + data);
   }

   return identifier;
}

std::optional<silo::FastaReader::SequenceIdentifier> silo::FastaReader::nextSkipGenome() {
   auto identifier = nextIdentifier();

   in_file.getInputStream().ignore(LONG_MAX, '\n');
   return identifier;
}

std::optional<silo::FastaReader::SequenceIdentifier> silo::FastaReader::next(std::string& genome_buffer) {
   auto identifier = nextIdentifier();
   if (!identifier) {
      return std::nullopt;
   }

   if (!getline(in_file.getInputStream(), genome_buffer)) {
      throw FastaFormatException("Missing genome sequence in line following identifier: " + identifier->key);
   }

   return identifier;
}

void silo::FastaReader::reset() {
   in_file.getInputStream().clear();                  // clear fail and eof bits
   in_file.getInputStream().seekg(0, std::ios::beg);  // g pointer back to the start
}
