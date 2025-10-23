#include "silo/common/aa_symbols.h"

#include <cstddef>
#include <string>

#include "silo/common/panic.h"
#include "silo/common/symbol_map.h"

namespace silo {

const silo::SymbolMap<AminoAcid, std::vector<AminoAcid::Symbol>> AminoAcid::AMBIGUITY_SYMBOLS{{{
   {Symbol::GAP, Symbol::X},
   {Symbol::A, Symbol::X},
   {Symbol::C, Symbol::X},
   {Symbol::D, Symbol::B, Symbol::X},
   {Symbol::E, Symbol::Z, Symbol::X},
   {Symbol::F, Symbol::X},
   {Symbol::G, Symbol::X},
   {Symbol::H, Symbol::X},
   {Symbol::I, Symbol::J, Symbol::X},
   {Symbol::K, Symbol::J, Symbol::X},
   {Symbol::L, Symbol::X},
   {Symbol::M, Symbol::X},
   {Symbol::N, Symbol::B, Symbol::X},
   {Symbol::O, Symbol::X},
   {Symbol::P, Symbol::X},
   {Symbol::Q, Symbol::Z, Symbol::X},
   {Symbol::R, Symbol::X},
   {Symbol::S, Symbol::X},
   {Symbol::T, Symbol::X},
   {Symbol::U, Symbol::X},
   {Symbol::V, Symbol::X},
   {Symbol::W, Symbol::X},
   {Symbol::Y, Symbol::X},
   {Symbol::B, Symbol::X},
   {Symbol::J, Symbol::X},
   {Symbol::Z, Symbol::X},
   {Symbol::STOP, Symbol::X},
   {Symbol::X},
}}};
}  // namespace silo
