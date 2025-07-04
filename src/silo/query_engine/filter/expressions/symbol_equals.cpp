#include "silo/query_engine/filter/expressions/symbol_equals.h"

#include <array>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/and.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/negation.h"
#include "silo/query_engine/filter/expressions/or.h"
#include "silo/query_engine/filter/operators/bitmap_selection.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
SymbolOrDot<SymbolType>::SymbolOrDot(typename SymbolType::Symbol symbol)
    : value(symbol) {}

template <typename SymbolType>
SymbolOrDot<SymbolType> SymbolOrDot<SymbolType>::dot() {
   return SymbolOrDot<SymbolType>();
}

template <typename SymbolType>
char SymbolOrDot<SymbolType>::asChar() const {
   return value.has_value() ? SymbolType::symbolToChar(value.value()) : '.';
}

template <typename SymbolType>
typename SymbolType::Symbol SymbolOrDot<SymbolType>::getSymbolOrReplaceDotWith(
   typename SymbolType::Symbol replace_dot_with
) const {
   return value.value_or(replace_dot_with);
}

template class SymbolOrDot<AminoAcid>;
template class SymbolOrDot<Nucleotide>;

template <typename SymbolType>
SymbolEquals<SymbolType>::SymbolEquals(
   std::optional<std::string> sequence_name,
   uint32_t position_idx,
   SymbolOrDot<SymbolType> value
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx),
      value(value) {}

template <typename SymbolType>
std::string SymbolEquals<SymbolType>::toString() const {
   return fmt::format(
      "{}{}{}", sequence_name ? sequence_name.value() + ":" : "", position_idx + 1, value.asChar()
   );
}

template <typename SymbolType>
std::unique_ptr<silo::query_engine::filter::operators::Operator> SymbolEquals<SymbolType>::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   Expression::AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || table.schema.getDefaultSequenceName<SymbolType>(),
      "Database does not have a default sequence name for {} sequences. "
      "You need to provide the sequence name with the {} filter.",
      SymbolType::SYMBOL_NAME,
      getFilterName()
   );

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, table.schema);

   const auto& seq_store_partition =
      table_partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   CHECK_SILO_QUERY(
      position_idx < seq_store_partition.metadata->reference_sequence.size(),
      "{} position is out of bounds {} > {}",
      getFilterName(),
      position_idx + 1,
      seq_store_partition.metadata->reference_sequence.size()
   )

   auto symbol = value.getSymbolOrReplaceDotWith(
      seq_store_partition.metadata->reference_sequence.at(position_idx)
   );
   if (mode == UPPER_BOUND) {
      auto symbols_to_match = SymbolType::AMBIGUITY_SYMBOLS.at(symbol);
      ExpressionVector symbol_filters;
      std::ranges::transform(
         symbols_to_match,
         std::back_inserter(symbol_filters),
         [&](typename SymbolType::Symbol symbol) {
            return std::make_unique<SymbolEquals<SymbolType>>(
               valid_sequence_name, position_idx, symbol
            );
         }
      );
      return Or(std::move(symbol_filters)).compile(table, table_partition, NONE);
   }
   if (symbol == SymbolType::SYMBOL_MISSING) {
      SPDLOG_TRACE(
         "Filtering for '{}' at position {}",
         SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING),
         position_idx
      );
      auto logical_equivalent = std::make_unique<SymbolEquals>(
         valid_sequence_name, position_idx, SymbolType::SYMBOL_MISSING
      );
      return std::make_unique<operators::BitmapSelection>(
         std::move(logical_equivalent),
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
      auto logical_equivalent_of_nested_index_scan = std::make_unique<Negation>(
         std::make_unique<SymbolEquals>(valid_sequence_name, position_idx, symbol)
      );
      return std::make_unique<operators::Complement>(
         std::make_unique<operators::IndexScan>(
            std::move(logical_equivalent_of_nested_index_scan),
            seq_store_partition.getBitmap(position_idx, symbol),
            table_partition.sequence_count
         ),
         table_partition.sequence_count
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
      std::erase(symbols, symbol);
      ExpressionVector symbol_filters;
      std::ranges::transform(
         symbols,
         std::back_inserter(symbol_filters),
         [&](typename SymbolType::Symbol symbol) {
            return std::make_unique<Negation>(
               std::make_unique<SymbolEquals<SymbolType>>(valid_sequence_name, position_idx, symbol)
            );
         }
      );
      return And(std::move(symbol_filters)).compile(table, table_partition, NONE);
   }
   SPDLOG_TRACE(
      "Filtering for symbol '{}' at position {}", SymbolType::symbolToChar(symbol), position_idx
   );
   auto logical_equivalent =
      std::make_unique<SymbolEquals>(valid_sequence_name, position_idx, symbol);
   return std::make_unique<operators::IndexScan>(
      std::move(logical_equivalent),
      seq_store_partition.getBitmap(position_idx, symbol),
      table_partition.sequence_count
   );
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<SymbolEquals<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("position"),
      "The field 'position' is required in a SymbolEquals expression"
   );
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in a SymbolEquals expression needs to be an unsigned integer"
   );
   CHECK_SILO_QUERY(
      json.contains("symbol"), "The field 'symbol' is required in a SymbolEquals expression"
   );
   CHECK_SILO_QUERY(
      json["symbol"].is_string(),
      "The field 'symbol' in a SymbolEquals expression needs to be a string"
   );
   std::optional<std::string> sequence_name;
   if (json.contains("sequenceName")) {
      sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position_idx_1_indexed = json["position"].get<uint32_t>();
   CHECK_SILO_QUERY(
      position_idx_1_indexed > 0, "The field 'position' is 1-indexed. Value of 0 not allowed."
   );
   const uint32_t position_idx = position_idx_1_indexed - 1;
   const std::string& symbol = json["symbol"];

   CHECK_SILO_QUERY(
      symbol.size() == 1, "The string field 'symbol' must be exactly one character long"
   );

   if (symbol.at(0) == '.') {
      filter = std::make_unique<SymbolEquals<SymbolType>>(
         sequence_name, position_idx, SymbolOrDot<SymbolType>::dot()
      );
      return;
   }
   const std::optional<typename SymbolType::Symbol> symbol_value =
      SymbolType::charToSymbol(symbol.at(0));
   CHECK_SILO_QUERY(
      symbol_value.has_value(),
      "The string field 'symbol' must be either a valid {} symbol or the '.' symbol.",
      SymbolType::SYMBOL_NAME
   );
   filter = std::make_unique<SymbolEquals<SymbolType>>(sequence_name, position_idx, *symbol_value);
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

}  // namespace silo::query_engine::filter::expressions
