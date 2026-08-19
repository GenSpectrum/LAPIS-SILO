#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace rhydb {

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

   static ReferenceGenomes readFromFile(const std::filesystem::path& reference_genomes_path);

   template <typename SymbolType>
   static std::vector<typename SymbolType::Symbol> stringToVector(const std::string& string) {
      std::vector<typename SymbolType::Symbol> sequence_vector;

      for (const char character : string) {
         auto symbol = SymbolType::charToSymbol(character);

         if (!symbol.has_value()) {
            throw std::runtime_error(fmt::format(
               "{} reference sequence with illegal {} code: {}",
               SymbolType::SYMBOL_NAME,
               SymbolType::SYMBOL_NAME_LOWER_CASE,
               std::to_string(character)
            ));
         }

         sequence_vector.push_back(*symbol);
      }
      return sequence_vector;
   }
};

}  // namespace rhydb
