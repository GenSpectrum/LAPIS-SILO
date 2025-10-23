#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"

namespace silo {

const silo::SymbolMap<Nucleotide, std::vector<Nucleotide::Symbol>> Nucleotide::AMBIGUITY_SYMBOLS{{{
   {Symbol::GAP},
   {Symbol::A, Symbol::R, Symbol::M, Symbol::W, Symbol::D, Symbol::H, Symbol::V, Symbol::N},
   {Symbol::C, Symbol::Y, Symbol::M, Symbol::S, Symbol::B, Symbol::H, Symbol::V, Symbol::N},
   {Symbol::G, Symbol::R, Symbol::K, Symbol::S, Symbol::B, Symbol::D, Symbol::V, Symbol::N},
   {Symbol::T, Symbol::Y, Symbol::K, Symbol::W, Symbol::B, Symbol::D, Symbol::H, Symbol::N},
   {Symbol::R},
   {Symbol::Y},
   {Symbol::S},
   {Symbol::W},
   {Symbol::K},
   {Symbol::M},
   {Symbol::B},
   {Symbol::D},
   {Symbol::H},
   {Symbol::V},
   {Symbol::N},
}}};
}  // namespace silo
