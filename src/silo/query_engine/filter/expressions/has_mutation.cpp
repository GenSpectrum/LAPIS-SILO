#include "silo/query_engine/filter/expressions/has_mutation.h"

#include <memory>
#include <utility>
#include <vector>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/expressions/symbol_in_set.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/table.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
HasMutation<SymbolType>::HasMutation(
   std::optional<std::string> sequence_name,
   uint32_t position_idx
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx) {}

template <typename SymbolType>
std::string HasMutation<SymbolType>::toString() const {
   const std::string sequence_name_prefix = sequence_name ? sequence_name.value() + ":" : "";
   return sequence_name_prefix + std::to_string(position_idx);
}

template <typename SymbolType>
std::unique_ptr<Expression> HasMutation<SymbolType>::rewrite(
   const storage::Table& table,
   AmbiguityMode mode
) const {
   CHECK_SILO_QUERY(
      sequence_name.has_value() || table.schema->getDefaultSequenceName<SymbolType>(),
      "Database does not have a default sequence name for {} Sequences. "
      "You need to provide the sequence name with the {}Mutation filter.",
      SymbolType::SYMBOL_NAME,
      SymbolType::SYMBOL_NAME
   );

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, *table.schema);

   const auto& sequence_column =
      table.columns.getColumns<typename SymbolType::Column>().at(valid_sequence_name);

   auto column_metadata =
      table.schema->getColumnMetadata<typename SymbolType::Column>(valid_sequence_name).value();
   CHECK_SILO_QUERY(
      position_idx < column_metadata->reference_sequence.size(),
      "Has{}Mutation position is out of bounds {} > {}",
      SymbolType::SYMBOL_NAME,
      position_idx + 1,
      sequence_column.metadata->reference_sequence.size()
   )

   auto ref_symbol = sequence_column.metadata->reference_sequence.at(position_idx);

   std::vector<typename SymbolType::Symbol> symbols =
      std::vector(SymbolType::SYMBOLS.begin(), SymbolType::SYMBOLS.end());
   if (mode == AmbiguityMode::UPPER_BOUND) {
      // We can only be sure, that the symbol did not mutate, if the ref_symbol is at that position
      std::erase(symbols, ref_symbol);
   } else {
      // Remove all symbols that could match the searched base
      for (const auto symbol : SymbolType::AMBIGUITY_SYMBOLS.at(ref_symbol)) {
         std::erase(symbols, symbol);
      }
   }
   return std::make_unique<SymbolInSet<SymbolType>>(
      valid_sequence_name, position_idx, std::move(symbols)
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> HasMutation<SymbolType>::compile(
   const storage::Table& /*table*/
) const {
   throw QueryCompilationException{
      "Has{}Mutation expression must be eliminated in query rewrite phase", SymbolType::SYMBOL_NAME
   };
}

template class HasMutation<AminoAcid>;
template class HasMutation<Nucleotide>;

}  // namespace silo::query_engine::filter::expressions
