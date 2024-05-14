#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "silo/sequence_file_reader/sequence_file_reader.h"

namespace silo::sequence_file_reader {

class SamReader : public SequenceFileReader {
  public:
   explicit SamReader(const std::filesystem::path& in_file_name)
       : SequenceFileReader(in_file_name) {}
   explicit SamReader(const std::string& file_content)
       : SequenceFileReader(file_content) {}
   std::optional<ReadSequence> nextEntry() override;
};
}  // namespace silo::sequence_file_reader
