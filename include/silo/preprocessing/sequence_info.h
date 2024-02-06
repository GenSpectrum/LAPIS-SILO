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
   std::vector<std::string> nuc_sequence_names;
   std::vector<std::string> aa_sequence_names;

  public:
   explicit SequenceInfo(const silo::ReferenceGenomes& reference_genomes);

   [[nodiscard]] std::vector<std::string> getAlignedSequenceSelects(
      const PreprocessingDatabase& preprocessing_db
   ) const;

   static std::string getNucleotideSequenceSelect(
      const std::string& seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   static std::string getUnalignedSequenceSelect(
      const std::string& seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   static std::string getAminoAcidSequenceSelect(
      const std::string& seq_name,
      const PreprocessingDatabase& preprocessing_db
   );

   void validate(duckdb::Connection& connection, const std::filesystem::path& input_filename) const;
};
}  // namespace preprocessing
}  // namespace silo