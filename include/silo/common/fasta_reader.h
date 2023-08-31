#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "silo/common/input_stream_wrapper.h"

namespace silo {
class FastaReader {
  private:
   silo::InputStreamWrapper in_file;

   std::optional<std::string> nextKey();

  public:
   explicit FastaReader(const std::filesystem::path& in_file_name);

   std::optional<std::string> nextSkipGenome();

   std::optional<std::string> next(std::string& genome_buffer);

   void reset();
};
}  // namespace silo
