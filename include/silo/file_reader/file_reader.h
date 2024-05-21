#pragma once

#include <optional>
#include <string>
#include "silo/common/input_stream_wrapper.h"

namespace silo {
class FileReader {

  protected:
   explicit FileReader(const std::filesystem::path& in_file_name)
       : in_file(in_file_name){};

   silo::InputStreamWrapper in_file;

  public:
   struct ReadSequence {
      std::string key;
      uint32_t offset;
      std::string sequence;
   };

   virtual std::optional<ReadSequence> nextEntry() = 0;

   void reset();

   virtual ~FileReader() {};
};
}  // namespace silo
