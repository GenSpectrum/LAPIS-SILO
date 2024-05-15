#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "silo/common/symbol_map.h"

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

   static constexpr uint32_t COUNT = 16;

   static constexpr std::string_view SYMBOL_NAME = "Nucleotide";
   static constexpr std::string_view SYMBOL_NAME_LOWER_CASE = "nucleotide";
   static constexpr std::string_view SYMBOL_NAME_UPPER_CASE = "NUCLEOTIDE";
   static constexpr std::string_view SYMBOL_NAME_SHORT = "NUC";
   static constexpr std::string_view PREFIX = "nuc_";

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

   static constexpr std::array<Symbol, 5> VALID_MUTATION_SYMBOLS{
      Nucleotide::Symbol::GAP,
      Nucleotide::Symbol::A,
      Nucleotide::Symbol::C,
      Nucleotide::Symbol::G,
      Nucleotide::Symbol::T,
   };

   static constexpr std::array<Symbol, 11> INVALID_MUTATION_SYMBOLS{
      Nucleotide::Symbol::R,  // A or G
      Nucleotide::Symbol::Y,  // C or T
      Nucleotide::Symbol::S,  // G or C
      Nucleotide::Symbol::W,  // A or T
      Nucleotide::Symbol::K,  // G or T
      Nucleotide::Symbol::M,  // A or C
      Nucleotide::Symbol::B,  // C or G or T
      Nucleotide::Symbol::D,  // A or G or T
      Nucleotide::Symbol::H,  // A or C or T
      Nucleotide::Symbol::V,  // A or C or G
      Nucleotide::Symbol::N,  // any base
   };

   static_assert(INVALID_MUTATION_SYMBOLS.size() + VALID_MUTATION_SYMBOLS.size() == SYMBOLS.size());

   static const silo::SymbolMap<Nucleotide, std::vector<Nucleotide::Symbol>> AMBIGUITY_SYMBOLS;

   static constexpr Symbol SYMBOL_MISSING = Symbol::N;

   static char symbolToChar(Symbol symbol);

   static std::optional<Symbol> charToSymbol(char character);

   static std::optional<std::vector<Symbol>> stringToSymbolVector(const std::string& sequence);

   static std::optional<char> findIllegalChar(const std::string& sequence);
};

}  // namespace silo
