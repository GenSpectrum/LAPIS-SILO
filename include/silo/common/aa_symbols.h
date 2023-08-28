#ifndef SILO_SymbolS_H
#define SILO_SymbolS_H

#include <array>
#include <iostream>
#include <optional>
#include <vector>

namespace silo {

class AminoAcid {
  public:
   /// https://www.bioinformatics.org/sms2/iupac.html
   enum class Symbol : char {
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
   static constexpr uint32_t COUNT = static_cast<uint32_t>(Symbol::X) + 1;

   static constexpr std::string_view SYMBOL_NAME = "Amino Acid";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "amino acid";
   static constexpr std::string_view SYMBOL_NAME_UPPER_CASE = "AMINO ACID";
   static constexpr std::string_view SYMBOL_NAME_SHORT = "AA";

   static constexpr std::array<Symbol, COUNT> SYMBOLS{
      Symbol::GAP, Symbol::A, Symbol::C, Symbol::D,    Symbol::E, Symbol::F, Symbol::G,
      Symbol::H,   Symbol::I, Symbol::K, Symbol::L,    Symbol::M, Symbol::N, Symbol::P,
      Symbol::Q,   Symbol::R, Symbol::S, Symbol::T,    Symbol::V, Symbol::W, Symbol::Y,
      Symbol::B,   Symbol::Z, Symbol::X, Symbol::STOP,
   };

   static char symbolToChar(Symbol symbol);

   static std::optional<Symbol> charToSymbol(char character);

   static std::optional<std::vector<Symbol>> stringToSymbolVector(const std::string& sequence);

   static std::optional<char> findIllegalChar(const std::string& sequence);
};
}  // namespace silo
// namespace silo

#endif  // SILO_SymbolS_H
