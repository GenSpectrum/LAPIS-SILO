#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"

namespace silo {

struct ReferenceGenomes {
   std::vector<std::string> nucleotide_sequence_names;
   std::vector<std::string> aa_sequence_names;
   std::vector<std::string> raw_nucleotide_sequences;
   std::vector<std::string> raw_aa_sequences;

   ReferenceGenomes() = default;

   explicit ReferenceGenomes(
      const std::vector<std::pair<std::string, std::string>>& nucleotide_sequences_,
      const std::vector<std::pair<std::string, std::string>>& aa_sequences_
   );

   void writeToFile(const std::filesystem::path& reference_genomes_path) const;

   static ReferenceGenomes readFromFile(const std::filesystem::path& reference_genomes_path);

   template <typename SymbolType>
   std::vector<std::string> getSequenceNames() const;

   template <typename SymbolType>
   std::vector<std::vector<typename SymbolType::Symbol>> getReferenceSequences() const;

   template <typename SymbolType>
   static std::vector<typename SymbolType::Symbol> stringToVector(const std::string& string);

   template <typename SymbolType>
   static std::string vectorToString(const std::vector<typename SymbolType::Symbol>& vector);
};

}  // namespace silo
