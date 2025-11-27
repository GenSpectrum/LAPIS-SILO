#include "silo/query_engine/filter/expressions/symbol_in_set.h"

#include <memory>
#include <utility>
#include <vector>

#include <fmt/ranges.h>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/intersection.h"
#include "silo/query_engine/filter/operators/is_in_covered_region.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/selection.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/query_engine/query_compilation_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"

using silo::storage::column::SequenceColumnPartition;

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
SymbolInSet<SymbolType>::SymbolInSet(
   std::optional<std::string> sequence_name,
   uint32_t position_idx,
   std::vector<typename SymbolType::Symbol> symbols
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx),
      symbols(std::move(symbols)) {}

template <typename SymbolType>
std::string SymbolInSet<SymbolType>::toString() const {
   std::string symbols_string = fmt::format(
      "{}",
      fmt::join(
         symbols | std::views::transform([](typename SymbolType::Symbol symbol) {
            return SymbolType::symbolToChar(symbol);
         }),
         ", "
      )
   );

   return fmt::format(
      "({}symbol at position {} in {{{}}})",
      sequence_name ? sequence_name.value() + ":" : "",
      position_idx + 1,
      symbols_string
   );
}

namespace {

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> negateSymbolsExcluding(
   const std::vector<typename SymbolType::Symbol>& symbols,
   typename SymbolType::Symbol excluded_symbol
) {
   std::vector<typename SymbolType::Symbol> result;
   for (const auto& symbol : SymbolType::SYMBOLS) {
      if (symbol == excluded_symbol) {
         continue;
      }
      if (std::find(symbols.begin(), symbols.end(), symbol) == symbols.end()) {
         result.push_back(symbol);
      }
   }
   return result;
}

template <typename SymbolType>
std::vector<typename SymbolType::Symbol> negateSymbols(
   const std::vector<typename SymbolType::Symbol>& symbols
) {
   std::vector<typename SymbolType::Symbol> result;
   for (const auto& symbol : SymbolType::SYMBOLS) {
      if (std::find(symbols.begin(), symbols.end(), symbol) == symbols.end()) {
         result.push_back(symbol);
      }
   }
   return result;
}

// To make the difference, we produce `Left & !Right`
std::unique_ptr<operators::Operator> makeDifference(
   std::unique_ptr<operators::Operator> left,
   std::unique_ptr<operators::Operator> right,
   uint32_t row_count
) {
   operators::OperatorVector non_negated_operators;
   non_negated_operators.push_back(std::move(left));
   operators::OperatorVector negated_operators;
   negated_operators.push_back(std::move(right));
   return std::make_unique<operators::Intersection>(
      std::move(non_negated_operators), std::move(negated_operators), row_count
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> compileWithMissingSymbolAndReference(
   const SequenceColumnPartition<SymbolType>& sequence_column_partition,
   uint32_t position_idx,
   const std::vector<typename SymbolType::Symbol>& symbols
) {
   // as the missing symbol and the reference symbol are included, we can just negate the other
   // symbols
   auto negated_symbols = negateSymbols<SymbolType>(symbols);
   auto bitmap = sequence_column_partition.vertical_sequence_index.getMatchingContainersAsBitmap(
      position_idx, negated_symbols
   );
   return std::make_unique<operators::Complement>(
      std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{std::move(bitmap)}, sequence_column_partition.sequence_count
      ),
      sequence_column_partition.sequence_count
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> compileWithMissingSymbol(
   const SequenceColumnPartition<SymbolType>& sequence_column_partition,
   uint32_t position_idx,
   const std::vector<typename SymbolType::Symbol>& symbols
) {
   // The missing symbol is included, so we start with the sequences with no coverage at this
   // position and then add the sequences with the mutation symbols
   auto bitmap = sequence_column_partition.vertical_sequence_index.getMatchingContainersAsBitmap(
      position_idx, symbols
   );

   operators::OperatorVector operators_for_union;
   operators_for_union.push_back(std::make_unique<operators::IsInCoveredRegion>(
      &sequence_column_partition.horizontal_coverage_index.start_end,
      &sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps,
      sequence_column_partition.sequence_count,
      operators::IsInCoveredRegion::Comparator::NOT_COVERED,
      position_idx
   ));
   operators_for_union.push_back(std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{std::move(bitmap)}, sequence_column_partition.sequence_count
   ));
   return std::make_unique<operators::Union>(
      std::move(operators_for_union), sequence_column_partition.sequence_count
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> compileWithReference(
   const SequenceColumnPartition<SymbolType>& sequence_column_partition,
   uint32_t position_idx,
   const std::vector<typename SymbolType::Symbol>& symbols
) {
   // The reference symbol is included, so we start with the sequences with coverage at this
   // position and then remove the sequences with the negated mutation symbols
   auto negated_symbols = negateSymbolsExcluding<SymbolType>(symbols, SymbolType::SYMBOL_MISSING);
   auto bitmap = sequence_column_partition.vertical_sequence_index.getMatchingContainersAsBitmap(
      position_idx, negated_symbols
   );

   return makeDifference(
      std::make_unique<operators::IsInCoveredRegion>(
         &sequence_column_partition.horizontal_coverage_index.start_end,
         &sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps,
         sequence_column_partition.sequence_count,
         operators::IsInCoveredRegion::Comparator::COVERED,
         position_idx
      ),
      std::make_unique<operators::IndexScan>(
         CopyOnWriteBitmap{std::move(bitmap)}, sequence_column_partition.sequence_count
      ),
      sequence_column_partition.sequence_count
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> compileOnlyMutations(
   const SequenceColumnPartition<SymbolType>& sequence_column_partition,
   uint32_t position_idx,
   const std::vector<typename SymbolType::Symbol>& symbols
) {
   // All our results are fully included in the vertical sequence index
   auto bitmap = sequence_column_partition.vertical_sequence_index.getMatchingContainersAsBitmap(
      position_idx, symbols
   );
   return std::make_unique<operators::IndexScan>(
      CopyOnWriteBitmap{std::move(bitmap)}, sequence_column_partition.sequence_count
   );
}

}  // namespace

template <typename SymbolType>
std::unique_ptr<Expression> SymbolInSet<SymbolType>::rewrite(
   const storage::Table& /*table*/,
   const storage::TablePartition& /*table_partition*/,
   AmbiguityMode /*mode*/
) const {
   throw QueryCompilationException(
      "Cannot rewrite SymbolInSet - this expression should only be created during query rewrites "
      "and not directly used"
   );
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> SymbolInSet<SymbolType>::compile(
   const storage::Table& table,
   const storage::TablePartition& table_partition
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

   auto local_reference_symbol = sequence_column_partition.getLocalReferencePosition(position_idx);
   bool includes_reference =
      std::find(symbols.begin(), symbols.end(), local_reference_symbol) != symbols.end();

   bool includes_missing_symbol =
      std::find(symbols.begin(), symbols.end(), SymbolType::SYMBOL_MISSING) != symbols.end();

   if (includes_reference && includes_missing_symbol) {
      return compileWithMissingSymbolAndReference(sequence_column_partition, position_idx, symbols);
   }
   if (includes_missing_symbol) {
      return compileWithMissingSymbol(sequence_column_partition, position_idx, symbols);
   }
   if (includes_reference) {
      return compileWithReference(sequence_column_partition, position_idx, symbols);
   }
   return compileOnlyMutations(sequence_column_partition, position_idx, symbols);
}

template class SymbolInSet<AminoAcid>;
template class SymbolInSet<Nucleotide>;

}  // namespace silo::query_engine::filter::expressions
