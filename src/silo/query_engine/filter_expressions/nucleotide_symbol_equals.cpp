#include "silo/query_engine/filter_expressions/nucleotide_symbol_equals.h"

#include <nlohmann/json.hpp>
#include <vector>

#include "silo/database.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/reference_genome.h"

namespace silo::query_engine::filter_expressions {

NucleotideSymbolEquals::NucleotideSymbolEquals(uint32_t position, char value)
    : position(position),
      value(value) {}

std::string NucleotideSymbolEquals::toString(const silo::Database& /*database*/) {
   std::string res = std::to_string(position + 1) + std::to_string(value);
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> NucleotideSymbolEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   NUCLEOTIDE_SYMBOL nucleotide_symbol;
   if (value == '.') {
      const char character = database.reference_genome->genome_segments[0].at(position);
      nucleotide_symbol = toNucleotideSymbol(character);
   } else {
      nucleotide_symbol = toNucleotideSymbol(value);
   }
   if (mode == UPPER_BOUND) {
      auto symbols_to_match = AMBIGUITY_SYMBOLS.at(static_cast<uint32_t>(nucleotide_symbol));
      std::vector<std::unique_ptr<Expression>> symbol_filters;
      std::transform(
         symbols_to_match.begin(),
         symbols_to_match.end(),
         std::back_inserter(symbol_filters),
         [&](silo::NUCLEOTIDE_SYMBOL symbol) {
            return std::make_unique<NucleotideSymbolEquals>(
               position, SYMBOL_REPRESENTATION[static_cast<uint32_t>(symbol)]
            );
         }
      );
      return std::make_unique<Or>(std::move(symbol_filters))
         ->compile(database, database_partition, NONE);
   }
   if (nucleotide_symbol == NUCLEOTIDE_SYMBOL::N && !database_partition.seq_store.positions[position].nucleotide_symbol_n_indexed) {
      return std::make_unique<operators::BitmapSelection>(
         database_partition.seq_store.nucleotide_symbol_n_bitmaps.data(),
         database_partition.seq_store.nucleotide_symbol_n_bitmaps.size(),
         operators::BitmapSelection::CONTAINS,
         position
      );
   }
   if (database_partition.seq_store.positions[position].symbol_whose_bitmap_is_flipped == nucleotide_symbol) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            database_partition.seq_store.getBitmap(position, nucleotide_symbol),
            database_partition.sequenceCount
         ),
         database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::IndexScan>(
      database_partition.seq_store.getBitmap(position, nucleotide_symbol),
      database_partition.sequenceCount
   );
}

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
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   const std::string& nucleotide_symbol = json["symbol"];
   filter = std::make_unique<NucleotideSymbolEquals>(position, nucleotide_symbol.at(0));
}

}  // namespace silo::query_engine::filter_expressions
