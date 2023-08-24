#ifndef SILO_AA_SYMBOLS_H
#define SILO_AA_SYMBOLS_H

#include <array>
#include <iostream>
#include <optional>
#include <vector>

#include "silo/common/symbols.h"

namespace silo {

/// https://www.bioinformatics.org/sms2/iupac.html
enum class AA_SYMBOL : char {
   GAP,   // - Gap in sequence
   A,     // Alanine
   C,     // Cysteine
   D,     // Aspartic Acid
   E,     // Glutamic Acid
   F,     // Phenylalanine
   G,     // Glycine
   H,     // Histidine
   I,     // Isoleucine
   K,     // Lysine
   L,     // Leucine
   M,     // Methionine
   N,     // Asparagine
   P,     // Proline
   Q,     // Glutamine
   R,     // Arginine
   S,     // Serine
   T,     // Threonine
   V,     // Valine
   W,     // Tryptophan
   Y,     // Tyrosine
   B,     // Aspartic acid or Asparagine
   Z,     // Glutamine or Glutamic acid
   STOP,  // Stop codon
   X,     // Any amino acid
};

template <>
class Util<AA_SYMBOL> {
  public:
   static constexpr uint32_t count = static_cast<uint32_t>(AA_SYMBOL::X) + 1;

   static constexpr std::string_view SYMBOL_NAME = "Amino Acid";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "amino acid";
   static constexpr std::string_view SYMBOL_NAME_UPPER_CASE = "AMINO ACID";
   static constexpr std::string_view SYMBOL_NAME_SHORT = "AA";

   static constexpr std::array<AA_SYMBOL, count> symbols{
      AA_SYMBOL::GAP, AA_SYMBOL::A, AA_SYMBOL::C, AA_SYMBOL::D, AA_SYMBOL::E,
      AA_SYMBOL::F,   AA_SYMBOL::G, AA_SYMBOL::H, AA_SYMBOL::I, AA_SYMBOL::K,
      AA_SYMBOL::L,   AA_SYMBOL::M, AA_SYMBOL::N, AA_SYMBOL::P, AA_SYMBOL::Q,
      AA_SYMBOL::R,   AA_SYMBOL::S, AA_SYMBOL::T, AA_SYMBOL::V, AA_SYMBOL::W,
      AA_SYMBOL::Y,   AA_SYMBOL::B, AA_SYMBOL::Z, AA_SYMBOL::X, AA_SYMBOL::STOP,
   };

   static char symbolToChar(AA_SYMBOL symbol);

   static std::optional<AA_SYMBOL> charToSymbol(char character);

   static std::optional<std::vector<AA_SYMBOL>> stringToSymbolVector(const std::string& sequence);

   static std::optional<char> findIllegalChar(const std::string& sequence);
};
}  // namespace silo
// namespace silo

#endif  // SILO_AA_SYMBOLS_H
