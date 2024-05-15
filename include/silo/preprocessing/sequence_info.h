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
   [[nodiscard]] static std::vector<std::string> getAlignedSequenceSelects(
      const silo::ReferenceGenomes& reference_genomes,
      const PreprocessingDatabase& preprocessing_db
   );

   [[nodiscard]] static std::string getNucleotideSequenceSelect(
      std::string_view seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   [[nodiscard]] static std::string getUnalignedSequenceSelect(
      std::string_view seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   [[nodiscard]] static std::string getAminoAcidSequenceSelect(
      std::string_view seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   static void validateNdjsonFile(
      const silo::ReferenceGenomes& reference_genomes,
      duckdb::Connection& connection,
      const std::filesystem::path& input_filename
   );
};
}  // namespace preprocessing
}  // namespace silo