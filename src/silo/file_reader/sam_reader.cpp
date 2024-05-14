#include <optional>
#include <string>

#include <silo/common/string_utils.h>
#include "silo/sequence_file_reader/sam_format_exception.h"
#include "silo/sequence_file_reader/sam_reader.h"
#include "silo/sequence_file_reader/sequence_file_reader.h"

namespace silo::sequence_file_reader {

std::optional<SequenceFileReader::ReadSequence> SamReader::nextEntry() {
   std::string data;
   if (!getline(in_file.getInputStream(), data)) {
      return std::nullopt;
   }

   // optional header data
   while ((data.empty() || data.at(0) == '@') && getline(in_file.getInputStream(), data)) {
      ;
   }

   auto parts = splitBy(data, "\t");
   if (parts.size() < 12) {
      throw SamFormatException(
         "Sam incorrectly formatted: 11 tab separated fields are required, incorrect row: " + data
      );
   }
   if (parts.at(0).empty()) {
      throw SamFormatException("Sam incorrectly formatted: missing key in row: " + data);
   }
   if (parts.at(3).empty()) {
      throw SamFormatException("Sam incorrectly formatted: missing offset in row: " + data);
   }
   if (parts.at(9).empty()) {
      throw SamFormatException("Sam incorrectly formatted: empty sequence in row: " + data);
   }
   return SequenceFileReader::ReadSequence{
      .key = parts.at(0),
      .offset = static_cast<uint32_t>(std::stoi(parts.at(3))),
      .sequence = parts.at(9)
   };
}
}  // namespace silo::sequence_file_reader