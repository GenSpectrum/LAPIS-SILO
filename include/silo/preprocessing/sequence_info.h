#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "silo/storage/reference_genomes.h"

namespace silo::preprocessing {

class SequenceInfo {
  public:
   static void validateNdjsonFile(
      const silo::ReferenceGenomes& reference_genomes,
      const std::filesystem::path& input_filename
   );
};
}  // namespace silo::preprocessing