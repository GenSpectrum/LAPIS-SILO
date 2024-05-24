#include <optional>
#include <string>

#include <silo/file_reader/file_reader.h>
#include <silo/file_reader/sam_reader.h>

std::optional<silo::FileReader::ReadSequence> silo::SamReader::nextEntry() {
   std::string data;
   if (!getline(in_file.getInputStream(), data)) {
      return std::nullopt;
   }

   // optional header data
   while (data.at(0) == '@' && getline(in_file.getInputStream(), data))
      ;

   auto parts = splitBy(data, "\t");
   return silo::FileReader::ReadSequence{
      parts.at(0), static_cast<uint32_t>(std::stoi(parts.at(3))), parts.at(9)
   };
}