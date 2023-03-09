#ifndef SILO_H
#define SILO_H

#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace silo {

static constexpr unsigned GENOME_LENGTH = 29903;

// https://www.bioinformatics.org/sms/iupac.html
enum GENOME_SYMBOL {
   gap,  // -, gap
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

static constexpr unsigned symbolCount = static_cast<unsigned>(GENOME_SYMBOL::N) + 1;

static constexpr char symbol_rep[symbolCount] = {'-', 'A', 'C', 'G', 'T', 'R', 'Y', 'S',
                                                 'W', 'K', 'M', 'B', 'D', 'H', 'V', 'N'};

static_assert(symbol_rep[static_cast<unsigned>(GENOME_SYMBOL::N)] == 'N');

inline GENOME_SYMBOL to_symbol(char c) {
   GENOME_SYMBOL s = GENOME_SYMBOL::gap;
   switch (c) {
      case '.':
      case '-':
         s = GENOME_SYMBOL::gap;
         break;
      case 'A':
         s = GENOME_SYMBOL::A;
         break;
      case 'C':
         s = GENOME_SYMBOL::C;
         break;
      case 'G':
         s = GENOME_SYMBOL::G;
         break;
      case 'T':
      case 'U':
         s = GENOME_SYMBOL::T;
         break;
      case 'R':
         s = GENOME_SYMBOL::R;
         break;
      case 'Y':
         s = GENOME_SYMBOL::Y;
         break;
      case 'S':
         s = GENOME_SYMBOL::S;
         break;
      case 'W':
         s = GENOME_SYMBOL::W;
         break;
      case 'K':
         s = GENOME_SYMBOL::K;
         break;
      case 'M':
         s = GENOME_SYMBOL::M;
         break;
      case 'B':
         s = GENOME_SYMBOL::B;
         break;
      case 'D':
         s = GENOME_SYMBOL::D;
         break;
      case 'H':
         s = GENOME_SYMBOL::H;
         break;
      case 'V':
         s = GENOME_SYMBOL::V;
         break;
      case 'N':
         s = GENOME_SYMBOL::N;
         break;
      default:
         std::cerr << "unrecognized symbol " << c << std::endl;
   }
   return s;
}

inline std::string resolve_alias(
   const std::unordered_map<std::string, std::string>& alias_key,
   const std::string& pango_lineage
) {
   std::string pango_pref;
   std::stringstream pango_lin_stream(pango_lineage);
   getline(pango_lin_stream, pango_pref, '.');
   if (alias_key.contains(pango_pref)) {
      if (pango_lin_stream.eof()) {
         return alias_key.at(pango_pref);
      }
      std::string x((std::istream_iterator<char>(pango_lin_stream)), std::istream_iterator<char>());
      return alias_key.at(pango_pref) + '.' + x;
   } else {
      return pango_lineage;
   }
}

static inline std::string chunk_string(unsigned partition, unsigned chunk) {
   return "P" + std::to_string(partition) + "_C" + std::to_string(chunk);
}

std::string number_fmt(unsigned long n);

}  // namespace silo

#endif  // SILO_H
