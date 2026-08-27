#pragma once

#include <istream>
#include <optional>
#include <string>

namespace rhydb::append::fasta {

/// One FASTA record: the identifier from its `>` header (the first whitespace-
/// delimited token) and the concatenation of its sequence lines.
struct FastaRecord {
   std::string identifier;
   std::string sequence;
};

/// Reads FASTA records from a text stream. Sequence lines are concatenated (line
/// wrapping and surrounding whitespace are removed); the header description after
/// the identifier is ignored.
class FastaReader {
  public:
   explicit FastaReader(std::istream& input);

   /// Parse and return the next record, or nullopt at end of input. Throws
   /// FastaException on malformed input (data before the first header, or a
   /// header with no identifier).
   std::optional<FastaRecord> next();

  private:
   std::istream* input;
   /// The identifier of the header line already read while finishing the previous
   /// record (a record ends only at the next '>' or EOF).
   std::optional<std::string> pending_identifier;
   bool reached_eof = false;
};

}  // namespace rhydb::append::fasta
