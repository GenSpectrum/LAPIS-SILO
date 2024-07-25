#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace duckdb {
class Connection;
}

namespace silo {

class ReferenceGenomes;

namespace preprocessing {

class PreprocessingDatabase;

class SequenceInfo {
  public:
   static void validateNdjsonFile(
      const silo::ReferenceGenomes& reference_genomes,
      const std::filesystem::path& input_filename
   );
};
}  // namespace preprocessing
}  // namespace silo