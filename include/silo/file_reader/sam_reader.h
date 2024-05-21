#pragma once

#include <filesystem>
#include <optional>
#include <string>

#include "../common/string_utils.h"
#include "fasta_format_exception.h"
#include "silo/common/input_stream_wrapper.h"

namespace silo {
class SamReader : public FileReader {
   public:
      std::optional<ReadSequence> nextEntry() override;
    explicit SamReader(const std::filesystem::path& in_file_name)
        : FileReader(in_file_name){}
};
}  // namespace silo
