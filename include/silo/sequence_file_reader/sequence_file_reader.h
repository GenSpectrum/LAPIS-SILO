#pragma once

#include <optional>
#include <string>

#include "silo/common/input_stream_wrapper.h"

namespace silo::sequence_file_reader {
class SequenceFileReader {
  protected:
   explicit SequenceFileReader(const std::filesystem::path& in_file_name)
       : in_file(in_file_name) {};
   explicit SequenceFileReader(const std::string& file_content)
       : in_file(file_content) {};

   silo::InputStreamWrapper in_file;

  public:
   struct ReadSequence {
      std::string key;
      uint32_t offset;
      std::string sequence;
   };

   virtual std::optional<ReadSequence> nextEntry() = 0;

   virtual ~SequenceFileReader() {};
};
}  // namespace silo::sequence_file_reader
