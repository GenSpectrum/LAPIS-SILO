#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace silo {

class Nucleotide {
  public:
   /// https://www.bioinformatics.org/sms2/iupac.html
   enum class Symbol : char {
      GAP,  // -, GAP
      A,    // Adenine
      C,    // Cytosine
      G,    // Guanine
      T,    // (or U) Thymine (or Uracil)
      R,    // A or G
      Y,    // C or T
      S,    // G or C
      W,    // A or T
      K,    // G or T
      M,    // A or C
      B,    // C or G or T
      D,    // A or G or T
      H,    // A or C or T
      V,    // A or C or G
      N,    // any base
   };

   static constexpr uint32_t COUNT = static_cast<uint32_t>(Symbol::N) + 1;

   static constexpr std::string_view SYMBOL_NAME = "Nucleotide";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "nucleotide";
   static constexpr std::string_view SYMBOL_NAME_UPPER_CASE = "NUCLEOTIDE";
   static constexpr std::string_view SYMBOL_NAME_SHORT = "NUC";

   static constexpr std::array<Symbol, COUNT> SYMBOLS{
      Symbol::GAP,
      Symbol::A,
      Symbol::C,
      Symbol::G,
      Symbol::T,
      Symbol::R,
      Symbol::Y,
      Symbol::S,
      Symbol::W,
      Symbol::K,
      Symbol::M,
      Symbol::B,
      Symbol::D,
      Symbol::H,
      Symbol::V,
      Symbol::N,
   };

   static constexpr Symbol SYMBOL_MISSING = Symbol::N;

   static char symbolToChar(Symbol symbol);

   static std::optional<Symbol> charToSymbol(char character);

   static std::optional<std::vector<Symbol>> stringToSymbolVector(const std::string& sequence);

   static std::optional<char> findIllegalChar(const std::string& sequence);
};

}  // namespace silo
