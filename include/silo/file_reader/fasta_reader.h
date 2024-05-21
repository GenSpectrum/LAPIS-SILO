#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "file_reader.h"

namespace silo {

class FastaReader : public FileReader {
   public:
      std::optional<ReadSequence> nextEntry() override;
    explicit FastaReader(const std::filesystem::path& in_file_name)
        : FileReader(in_file_name){}
};
}  // namespace silo