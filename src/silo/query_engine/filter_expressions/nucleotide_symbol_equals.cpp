#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/sequence_store.h"

using silo::NUCLEOTIDE_SYMBOL;

namespace {

const std::array<std::vector<NUCLEOTIDE_SYMBOL>, silo::NUC_SYMBOL_COUNT> AMBIGUITY_NUC_SYMBOLS{{
   {NUCLEOTIDE_SYMBOL::GAP},
   {NUCLEOTIDE_SYMBOL::A,
    NUCLEOTIDE_SYMBOL::R,
    NUCLEOTIDE_SYMBOL::M,
    NUCLEOTIDE_SYMBOL::W,
    NUCLEOTIDE_SYMBOL::D,
    NUCLEOTIDE_SYMBOL::H,
    NUCLEOTIDE_SYMBOL::V,
    NUCLEOTIDE_SYMBOL::N},
   {NUCLEOTIDE_SYMBOL::C,
    NUCLEOTIDE_SYMBOL::Y,
    NUCLEOTIDE_SYMBOL::M,
    NUCLEOTIDE_SYMBOL::S,
    NUCLEOTIDE_SYMBOL::B,
    NUCLEOTIDE_SYMBOL::H,
    NUCLEOTIDE_SYMBOL::V,
    NUCLEOTIDE_SYMBOL::N},
   {NUCLEOTIDE_SYMBOL::G,
    NUCLEOTIDE_SYMBOL::R,
    NUCLEOTIDE_SYMBOL::K,
    NUCLEOTIDE_SYMBOL::S,
    NUCLEOTIDE_SYMBOL::B,
    NUCLEOTIDE_SYMBOL::D,
    NUCLEOTIDE_SYMBOL::V,
    NUCLEOTIDE_SYMBOL::N},
   {NUCLEOTIDE_SYMBOL::T,
    NUCLEOTIDE_SYMBOL::Y,
    NUCLEOTIDE_SYMBOL::K,
    NUCLEOTIDE_SYMBOL::W,
    NUCLEOTIDE_SYMBOL::B,
    NUCLEOTIDE_SYMBOL::D,
    NUCLEOTIDE_SYMBOL::H,
    NUCLEOTIDE_SYMBOL::N},
   {NUCLEOTIDE_SYMBOL::R},
   {NUCLEOTIDE_SYMBOL::Y},
   {NUCLEOTIDE_SYMBOL::S},
   {NUCLEOTIDE_SYMBOL::W},
   {NUCLEOTIDE_SYMBOL::K},
   {NUCLEOTIDE_SYMBOL::M},
   {NUCLEOTIDE_SYMBOL::B},
   {NUCLEOTIDE_SYMBOL::D},
   {NUCLEOTIDE_SYMBOL::H},
   {NUCLEOTIDE_SYMBOL::V},
   {NUCLEOTIDE_SYMBOL::N},
}};
};  // namespace

namespace silo::query_engine::filter_expressions {

NucleotideSymbolEquals::NucleotideSymbolEquals(
   std::optional<std::string> nuc_sequence_name,
   uint32_t position,
   std::optional<NUCLEOTIDE_SYMBOL> value
)
    : nuc_sequence_name(std::move(nuc_sequence_name)),
      position(position),
      value(value) {}

std::string NucleotideSymbolEquals::toString(const silo::Database& /*database*/) const {
   const std::string nuc_sequence_name_prefix =
      nuc_sequence_name ? nuc_sequence_name.value() + ":" : "";
   const char symbol_char = value.has_value() ? nucleotideSymbolToChar(*value) : '.';
   return nuc_sequence_name_prefix + std::to_string(position + 1) + std::to_string(symbol_char);
}

std::unique_ptr<silo::query_engine::operators::Operator> NucleotideSymbolEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )
   const auto& seq_store_partition =
      database_partition.nuc_sequences.at(nuc_sequence_name_or_default);
   if (position >= seq_store_partition.reference_genome.size()) {
      throw QueryParseException(
         "NucleotideEquals position is out of bounds '" + std::to_string(position + 1) + "' > '" +
         std::to_string(seq_store_partition.reference_genome.size()) + "'"
      );
   }
   const NUCLEOTIDE_SYMBOL nucleotide_symbol =
      value.value_or(seq_store_partition.reference_genome.at(position));
   if (mode == UPPER_BOUND) {
      auto symbols_to_match = AMBIGUITY_NUC_SYMBOLS.at(static_cast<uint32_t>(nucleotide_symbol));
      std::vector<std::unique_ptr<Expression>> symbol_filters;
      std::transform(
         symbols_to_match.begin(),
         symbols_to_match.end(),
         std::back_inserter(symbol_filters),
         [&](silo::NUCLEOTIDE_SYMBOL symbol) {
            return std::make_unique<NucleotideSymbolEquals>(
               nuc_sequence_name_or_default, position, symbol
            );
         }
      );
      return std::make_unique<Or>(std::move(symbol_filters))
         ->compile(database, database_partition, NONE);
   }
   if (nucleotide_symbol == NUCLEOTIDE_SYMBOL::N) {
      return std::make_unique<operators::BitmapSelection>(
         seq_store_partition.nucleotide_symbol_n_bitmaps.data(),
         seq_store_partition.nucleotide_symbol_n_bitmaps.size(),
         operators::BitmapSelection::CONTAINS,
         position
      );
   }
   if (seq_store_partition.positions[position].symbol_whose_bitmap_is_flipped == nucleotide_symbol) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            seq_store_partition.getBitmap(position, nucleotide_symbol),
            database_partition.sequenceCount
         ),
         database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::IndexScan>(
      seq_store_partition.getBitmap(position, nucleotide_symbol), database_partition.sequenceCount
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<NucleotideSymbolEquals>& filter) {
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a NucleotideEquals expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && json["position"].get<uint32_t>() > 0,
      "The field 'position' in a NucleotideEquals expression needs to be an unsigned "
      "integer greater than 0"
   )
   CHECK_SILO_QUERY(
      json.contains("symbol"), "The field 'symbol' is required in a NucleotideEquals expression"
   )
   CHECK_SILO_QUERY(
      json["symbol"].is_string(),
      "The field 'symbol' in a NucleotideEquals expression needs to be a string"
   )
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   const std::string& nucleotide_symbol = json["symbol"];

   CHECK_SILO_QUERY(
      nucleotide_symbol.size() == 1, "The string field 'symbol' must be exactly one character long"
   )
   const std::optional<NUCLEOTIDE_SYMBOL> nuc_value =
      charToNucleotideSymbol(nucleotide_symbol.at(0));
   CHECK_SILO_QUERY(
      nuc_value.has_value() || nucleotide_symbol.at(0) == '.',
      "The string field 'symbol' must be either a valid nucleotide symbol or the '.' symbol."
   )

   filter = std::make_unique<NucleotideSymbolEquals>(nuc_sequence_name, position, nuc_value);
}

}  // namespace silo::query_engine::filter_expressions
