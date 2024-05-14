#include <string>

#include "silo/common/string_utils.h"
#include "silo/sequence_file_reader/fasta_format_exception.h"
#include "silo/sequence_file_reader/fasta_reader.h"

namespace silo::sequence_file_reader {

std::optional<SequenceFileReader::ReadSequence> FastaReader::nextEntry() {
   std::string data;
   while (data.empty()) {
      if (!getline(in_file.getInputStream(), data)) {
         return std::nullopt;
      }
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
      .key = key,
      .offset = fields.empty() ? 0 : static_cast<uint32_t>(std::stoi(fields[0])),
      .sequence = sequence
   };
}

}  // namespace silo::sequence_file_reader
