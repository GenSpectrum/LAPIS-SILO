#ifndef SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
#define SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_

#include <filesystem>
#include <string>
#include <vector>

namespace silo {

struct ReferenceGenomes {
   std::unordered_map<std::string, std::string> nucleotide_sequences;
   std::unordered_map<std::string, std::string> aa_sequences;

   ReferenceGenomes() = default;

   explicit ReferenceGenomes(
      std::unordered_map<std::string, std::string> nucleotide_sequences,
      std::unordered_map<std::string, std::string> aa_sequences
   );

   static ReferenceGenomes readFromFile(const std::filesystem::path& reference_genome_file);
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
