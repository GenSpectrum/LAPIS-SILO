#include "silo/query_engine/filter_expressions/symbol_equals.h"

#include <array>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/filter_expressions/and.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/filter_expressions/negation.h"
#include "silo/query_engine/filter_expressions/or.h"
#include "silo/query_engine/operators/bitmap_selection.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/index_scan.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

template <typename SymbolType>
SymbolEquals<SymbolType>::SymbolEquals(
   std::optional<std::string> sequence_name,
   uint32_t position_idx,
   std::optional<typename SymbolType::Symbol> value
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx),
      value(value) {}

template <typename SymbolType>
std::string SymbolEquals<SymbolType>::toString(const silo::Database& /*database*/) const {
   return fmt::format(
      "{}{}{}",
      sequence_name ? sequence_name.value() + ":" : "",
      position_idx + 1,
      value.has_value() ? SymbolType::symbolToChar(*value) : '.'
   );
}

template <typename SymbolType>
std::unique_ptr<silo::query_engine::operators::Operator> SymbolEquals<SymbolType>::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || database.getDefaultSequenceName<SymbolType>().has_value(),
      fmt::format(
         "Database does not have a default sequence name for {} Sequences", SymbolType::SYMBOL_NAME
      )
   );
   const std::string sequence_name_or_default =
      sequence_name.has_value() ? sequence_name.value()
                                : database.getDefaultSequenceName<SymbolType>().value();
   CHECK_SILO_QUERY(
      database.getSequenceStores<SymbolType>().contains(sequence_name_or_default),
      fmt::format(
         "Database does not contain the {} Sequence with name: '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name_or_default
      )
   )
   const auto& seq_store_partition =
      database_partition.getSequenceStores<SymbolType>().at(sequence_name_or_default);
   if (position_idx >= seq_store_partition.reference_sequence.size()) {
      throw QueryParseException(
         "SymbolEquals position is out of bounds '" + std::to_string(position_idx + 1) + "' > '" +
         std::to_string(seq_store_partition.reference_sequence.size()) + "'"
      );
   }
   auto symbol = value.value_or(seq_store_partition.reference_sequence.at(position_idx));
   if (mode == UPPER_BOUND) {
      auto symbols_to_match = SymbolType::AMBIGUITY_SYMBOLS.at(symbol);
      std::vector<std::unique_ptr<Expression>> symbol_filters;
      std::transform(
         symbols_to_match.begin(),
         symbols_to_match.end(),
         std::back_inserter(symbol_filters),
         [&](SymbolType::Symbol symbol) {
            return std::make_unique<SymbolEquals<SymbolType>>(
               sequence_name_or_default, position_idx, symbol
            );
         }
      );
      return Or(std::move(symbol_filters)).compile(database, database_partition, NONE);
   }
   if (symbol == SymbolType::SYMBOL_MISSING) {
      SPDLOG_TRACE(
         "Filtering for '{}' at position {}",
         SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING),
         position_idx
      );
      return std::make_unique<operators::BitmapSelection>(
         seq_store_partition.missing_symbol_bitmaps.data(),
         seq_store_partition.missing_symbol_bitmaps.size(),
         operators::BitmapSelection::CONTAINS,
         position_idx
      );
   }
   if (seq_store_partition.positions[position_idx].isSymbolFlipped(symbol)) {
      SPDLOG_TRACE(
         "Filtering for flipped symbol '{}' at position {}",
         SymbolType::symbolToChar(symbol),
         position_idx
      );
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            seq_store_partition.getBitmap(position_idx, symbol), database_partition.sequence_count
         ),
         database_partition.sequence_count
      );
   }
   if (seq_store_partition.positions[position_idx].isSymbolDeleted(symbol)) {
      SPDLOG_TRACE(
         "Filtering for deleted symbol '{}' at position {}",
         SymbolType::symbolToChar(symbol),
         position_idx
      );
      std::vector<typename SymbolType::Symbol> symbols = std::vector<typename SymbolType::Symbol>(
         SymbolType::SYMBOLS.begin(), SymbolType::SYMBOLS.end()
      );
      symbols.erase(std::remove(symbols.begin(), symbols.end(), symbol), symbols.end());
      std::vector<std::unique_ptr<filter_expressions::Expression>> symbol_filters;
      std::transform(
         symbols.begin(),
         symbols.end(),
         std::back_inserter(symbol_filters),
         [&](typename SymbolType::Symbol symbol) {
            return std::make_unique<Negation>(std::make_unique<SymbolEquals<SymbolType>>(
               sequence_name_or_default, position_idx, symbol
            ));
         }
      );
      return And(std::move(symbol_filters)).compile(database, database_partition, NONE);
   }
   SPDLOG_TRACE(
      "Filtering for symbol '{}' at position {}", SymbolType::symbolToChar(symbol), position_idx
   );
   return std::make_unique<operators::IndexScan>(
      seq_store_partition.getBitmap(position_idx, symbol), database_partition.sequence_count
   );
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<SymbolEquals<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a SymbolEquals expression"
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned() && json["position"].get<uint32_t>() > 0,
      "The field 'position' in a SymbolEquals expression needs to be an unsigned "
      "integer greater than 0"
   )
   CHECK_SILO_QUERY(
      json.contains("symbol"), "The field 'symbol' is required in a SymbolEquals expression"
   )
   CHECK_SILO_QUERY(
      json["symbol"].is_string(),
      "The field 'symbol' in a SymbolEquals expression needs to be a string"
   )
   std::optional<std::string> sequence_name;
   if (json.contains("sequenceName")) {
      sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position_idx = json["position"].get<uint32_t>() - 1;
   const std::string& symbol = json["symbol"];

   CHECK_SILO_QUERY(
      symbol.size() == 1, "The string field 'symbol' must be exactly one character long"
   )
   const std::optional<typename SymbolType::Symbol> symbol_value =
      SymbolType::charToSymbol(symbol.at(0));
   CHECK_SILO_QUERY(
      symbol_value.has_value() || symbol.at(0) == '.',
      fmt::format(
         "The string field 'symbol' must be either a valid {} symbol or the '.' symbol.",
         SymbolType::SYMBOL_NAME
      )
   )

   filter = std::make_unique<SymbolEquals<SymbolType>>(sequence_name, position_idx, symbol_value);
}

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<SymbolEquals<Nucleotide>>& filter
);

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<SymbolEquals<AminoAcid>>& filter
);

template class SymbolEquals<AminoAcid>;
template class SymbolEquals<Nucleotide>;

}  // namespace silo::query_engine::filter_expressions