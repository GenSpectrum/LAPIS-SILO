#include "silo/query_engine/filter/expressions/symbol_equals.h"

#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"

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
   AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || table.schema->getDefaultSequenceName<SymbolType>(),
      "Database does not have a default sequence name for {} sequences. "
      "You need to provide the sequence name with the {} filter.",
      SymbolType::SYMBOL_NAME,
      getFilterName()
   );

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, *table.schema);

   const auto& sequence_column =
      table.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   CHECK_SILO_QUERY(
      position_idx < sequence_column.metadata->reference_sequence.size(),
      "{} position is out of bounds {} > {}",
      getFilterName(),
      position_idx + 1,
      sequence_column.metadata->reference_sequence.size()
   );

   auto symbol =
      value.getSymbolOrReplaceDotWith(sequence_column.metadata->reference_sequence.at(position_idx)
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
   const storage::Table& /*table*/
) const {
   throw QueryCompilationException("SymbolEquals should have been rewritten before compilation");
}

template class SymbolEquals<AminoAcid>;
template class SymbolEquals<Nucleotide>;

}  // namespace silo::query_engine::filter::expressions
