#include "silo/common/aa_symbols.h"

#include <cstddef>
#include <string>

#include "silo/common/panic.h"
#include "silo/common/symbol_map.h"

namespace silo {

const silo::SymbolMap<AminoAcid, std::vector<AminoAcid::Symbol>> AminoAcid::AMBIGUITY_SYMBOLS{{{
   {AminoAcid::Symbol::GAP, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::A, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::C, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::D, AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::E, AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::F, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::G, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::H, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::I, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::K, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::L, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::M, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::N, AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::P, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Q, AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::R, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::S, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::T, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::V, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::W, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Y, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::B, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::Z, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::STOP, AminoAcid::Symbol::X},
   {AminoAcid::Symbol::X},
}}};
}  // namespace silo
