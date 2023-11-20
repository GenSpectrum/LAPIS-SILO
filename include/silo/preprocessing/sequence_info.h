#pragma once

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
   SequenceInfo(const silo::ReferenceGenomes& reference_genomes);

   std::vector<std::string>& getNucSequenceNames();

   std::vector<std::string>& getAASequenceNames();

   std::vector<std::string> getSequenceSelects();

   std::string getNucInsertionSelect();

   std::string getAAInsertionSelect();

   void validate(duckdb::Connection& connection, std::string_view input_filename) const;
};
}  // namespace preprocessing
}  // namespace silo