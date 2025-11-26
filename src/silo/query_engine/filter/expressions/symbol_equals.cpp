#include "silo/query_engine/filter/expressions/symbol_equals.h"

#include <utility>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_compilation_exception.h"
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
std::unique_ptr<Expression> SymbolEquals<SymbolType>::rewrite(
   const storage::Table& table,
   const storage::TablePartition& table_partition,
   AmbiguityMode mode
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

   const auto& sequence_column_partition =
      table_partition.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   CHECK_SILO_QUERY(
      position_idx < sequence_column_partition.metadata->reference_sequence.size(),
      "{} position is out of bounds {} > {}",
      getFilterName(),
      position_idx + 1,
      sequence_column_partition.metadata->reference_sequence.size()
   );

   auto symbol = value.getSymbolOrReplaceDotWith(
      sequence_column_partition.metadata->reference_sequence.at(position_idx)
   );
   if (mode == UPPER_BOUND) {
      auto symbols_to_match = SymbolType::AMBIGUITY_SYMBOLS.at(symbol);
      return std::make_unique<SymbolInSet<SymbolType>>(
         valid_sequence_name, position_idx, symbols_to_match
      );
   }

   return std::make_unique<SymbolInSet<SymbolType>>(
      valid_sequence_name, position_idx, std::vector<typename SymbolType::Symbol>{symbol}
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> SymbolEquals<SymbolType>::compile(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/
) const {
   throw QueryCompilationException("SymbolEquals should have been rewritten before compilation");
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
   const std::optional<typename SymbolType::Symbol> symbol_char =
      SymbolType::charToSymbol(symbol.at(0));
   CHECK_SILO_QUERY(
      symbol_char.has_value(),
      "The string field 'symbol' must be either a valid {} symbol or the '.' symbol.",
      SymbolType::SYMBOL_NAME
   );
   filter = std::make_unique<SymbolEquals<SymbolType>>(
      sequence_name, position_idx, SymbolOrDot<SymbolType>{symbol_char.value()}
   );
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
