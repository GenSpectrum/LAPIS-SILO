#ifndef SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
#define SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo {

struct ReferenceGenomes {
   std::map<std::string, std::vector<Nucleotide::Symbol>> nucleotide_sequences;
   std::map<std::string, std::vector<AminoAcid::Symbol>> aa_sequences;
   std::map<std::string, std::string> raw_nucleotide_sequences;
   std::map<std::string, std::string> raw_aa_sequences;

   ReferenceGenomes() = default;

   explicit ReferenceGenomes(
      std::map<std::string, std::string>&& raw_nucleotide_sequences_,
      std::map<std::string, std::string>&& raw_aa_sequences_
   );

   void writeToFile(const std::filesystem::path& reference_genomes_path) const;

   static ReferenceGenomes readFromFile(const std::filesystem::path& reference_genomes_path);
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
