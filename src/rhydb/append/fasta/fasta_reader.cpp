#include "rhydb/append/fasta/fasta_reader.h"

#include <cctype>

#include "rhydb/append/fasta/fasta_exception.h"

namespace rhydb::append::fasta {

namespace {

void stripTrailingCarriageReturn(std::string& line) {
   if (!line.empty() && line.back() == '\r') {
      line.pop_back();
   }
}

/// The identifier is the first whitespace-delimited token after '>'.
std::string parseIdentifier(const std::string& header_line) {
   size_t start = 1;  // skip '>'
   while (start < header_line.size() &&
          (std::isspace(static_cast<unsigned char>(header_line[start])) != 0)) {
      ++start;
   }
   size_t end = start;
   while (end < header_line.size() &&
          (std::isspace(static_cast<unsigned char>(header_line[end])) == 0)) {
      ++end;
   }
   if (end == start) {
      throw FastaException("FASTA header line has no identifier: '{}'", header_line);
   }
   return header_line.substr(start, end - start);
}

}  // namespace

FastaReader::FastaReader(std::istream& input)
    : input(&input) {}

std::optional<FastaRecord> FastaReader::next() {
   std::string line;

   // Establish the header of the record we are about to return: either the one
   // stashed while finishing the previous record, or the next header in the file.
   if (!pending_identifier.has_value()) {
      if (reached_eof) {
         return std::nullopt;
      }
      while (std::getline(*input, line)) {
         stripTrailingCarriageReturn(line);
         if (line.empty()) {
            continue;
         }
         if (line[0] != '>') {
            throw FastaException(
               "FASTA sequence data appears before the first '>' header line: '{}'", line
            );
         }
         pending_identifier = parseIdentifier(line);
         break;
      }
      if (!pending_identifier.has_value()) {
         reached_eof = true;  // empty (or all-blank) input
         return std::nullopt;
      }
   }

   FastaRecord record;
   record.identifier = *pending_identifier;
   pending_identifier.reset();

   while (std::getline(*input, line)) {
      stripTrailingCarriageReturn(line);
      if (!line.empty() && line[0] == '>') {
         pending_identifier = parseIdentifier(line);
         return record;
      }
      for (char character : line) {
         if (std::isspace(static_cast<unsigned char>(character)) == 0) {
            record.sequence.push_back(character);
         }
      }
   }

   reached_eof = true;
   return record;
}

}  // namespace rhydb::append::fasta
