#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "silo/sequence_file_reader/sequence_file_reader.h"

namespace silo::sequence_file_reader {

class FastaReader : public SequenceFileReader {
  public:
   explicit FastaReader(const std::filesystem::path& in_file_name)
       : SequenceFileReader(in_file_name) {}
   std::optional<ReadSequence> nextEntry() override;
};
}  // namespace silo::sequence_file_reader