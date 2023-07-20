#ifndef SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
#define SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace silo {

enum class NUCLEOTIDE_SYMBOL : char;
enum class AA_SYMBOL : char;

struct ReferenceGenomes {
   std::unordered_map<std::string, std::vector<NUCLEOTIDE_SYMBOL>> nucleotide_sequences;
   std::unordered_map<std::string, std::vector<AA_SYMBOL>> aa_sequences;
   std::unordered_map<std::string, std::string> raw_nucleotide_sequences;
   std::unordered_map<std::string, std::string> raw_aa_sequences;

   ReferenceGenomes() = default;

   explicit ReferenceGenomes(
      std::unordered_map<std::string, std::string> raw_nucleotide_sequences_,
      std::unordered_map<std::string, std::string> raw_aa_sequences_
   );

   void writeToFile(const std::filesystem::path& reference_genomes_path) const;

   static ReferenceGenomes readFromFile(const std::filesystem::path& reference_genomes_path);
};

}  // namespace silo

#endif  // SILO_INCLUDE_SILO_STORAGE_REFERENCE_GENOME_H_
