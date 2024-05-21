#include <silo/common/string_utils.h>
#include <silo/file_reader/fasta_format_exception.h>
#include <string>

#include "silo/file_reader/fasta_reader.h"

std::optional<silo::FileReader::ReadSequence> silo::FastaReader::nextEntry() {
   std::string data;
   if (!getline(in_file.getInputStream(), data)) {
      return std::nullopt;
   }

   if (data.empty() || data.at(0) != '>') {
      throw FastaFormatException("Fasta key prefix '>' missing for key: " + data);
   }

   auto parts = splitBy(data.substr(1), "|");
   auto key = parts.front();
   auto fields = std::vector<std::string>(parts.begin() + 1, parts.end());

   if (key.empty()) {
      throw FastaFormatException("Fasta description not valid, missing id: " + data);
   }

   std::string sequence;

   if (!getline(in_file.getInputStream(), sequence)) {
      throw FastaFormatException("Missing genome sequence in line following identifier: " + key);
   }

   return ReadSequence{
      key,
      fields.empty() ? 0 : static_cast<uint32_t>(std::stoi(fields[0])),
      sequence
   };
}