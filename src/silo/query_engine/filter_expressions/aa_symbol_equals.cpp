#include "silo/query_engine/filter_expressions/aa_symbol_equals.h"

#include <nlohmann/json.hpp>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

AASymbolEquals::AASymbolEquals(std::string aa_sequence_name, uint32_t position, char value)
    : aa_sequence_name(std::move(aa_sequence_name)),
      position(position),
      value(value) {}

std::string AASymbolEquals::toString(const silo::Database& /*database*/) const {
   std::string res = std::to_string(position + 1) + std::to_string(value);
   return res;
}

std::unique_ptr<silo::query_engine::operators::Operator> AASymbolEquals::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   const auto& aa_store_partition = database_partition.aa_sequences.at(aa_sequence_name);
   if (position >= aa_store_partition.reference_sequence.length()) {
      throw QueryParseException(
         "AminoAcidEquals position is out of bounds '" + std::to_string(position + 1) + "' > '" +
         std::to_string(aa_store_partition.reference_sequence.length()) + "'"
      );
   }
   AA_SYMBOL aa_symbol;
   if (value == '.') {
      const char character = aa_store_partition.reference_sequence.at(position);
      aa_symbol = toAASymbol(character).value_or(AA_SYMBOL::X);
   } else {
      aa_symbol = toAASymbol(value).value_or(AA_SYMBOL::X);
   }
   if (aa_symbol == AA_SYMBOL::X) {
      return std::make_unique<operators::BitmapSelection>(
         aa_store_partition.aa_symbol_x_bitmaps.data(),
         aa_store_partition.aa_symbol_x_bitmaps.size(),
         operators::BitmapSelection::CONTAINS,
         position
      );
   }
   if (aa_store_partition.positions[position].symbol_whose_bitmap_is_flipped == aa_symbol) {
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            aa_store_partition.getBitmap(position, aa_symbol), database_partition.sequenceCount
         ),
         database_partition.sequenceCount
      );
   }
   return std::make_unique<operators::IndexScan>(
      aa_store_partition.getBitmap(position, aa_symbol), database_partition.sequenceCount
   );
}

void from_json(const nlohmann::json& json, std::unique_ptr<AASymbolEquals>& filter) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") && json["sequenceName"].is_string(),
      "AminoAcidEquals expression requires the string field sequenceName"
      "integer"
   )
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a AminoAcidEquals expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && json["position"].get<uint32_t>() > 0,
      "The field 'position' in a AminoAcidEquals expression needs to be an unsigned "
      "integer greater than 0"
   )
   CHECK_SILO_QUERY(
      json.contains("symbol") && json["symbol"].is_string(),
      "The string field 'symbol' is required in a AminoAcidEquals expression"
   )
   const std::string aa_sequence_name = json["sequenceName"].get<std::string>();
   const uint32_t position = json["position"].get<uint32_t>() - 1;
   const std::string nucleotide_symbol = json["symbol"].get<std::string>();
   filter = std::make_unique<AASymbolEquals>(aa_sequence_name, position, nucleotide_symbol.at(0));
}

}  // namespace silo::query_engine::filter_expressions
