#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "fasta_format_exception.h"
#include "silo/common/input_stream_wrapper.h"
#include "string_utils.h"

namespace silo {
class FastaReader {
   public: struct SequenceIdentifier {
       std::string key;
       std::vector<std::string> fields;

       SequenceIdentifier(std::string line) {
          auto parts = splitBy(line, "|");
          key = parts.front();
          fields = std::vector<std::string>(parts.begin() + 1, parts.end());
       }
    };


  private:
   silo::InputStreamWrapper in_file;

   std::optional<SequenceIdentifier> nextIdentifier();

  public:
   explicit FastaReader(const std::filesystem::path& in_file_name);

   std::optional<silo::FastaReader::SequenceIdentifier> nextSkipGenome();

   std::optional<silo::FastaReader::SequenceIdentifier> next(std::string& genome_buffer);

   void reset();
};
}  // namespace silo
