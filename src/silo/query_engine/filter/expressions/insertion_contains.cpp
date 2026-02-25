#include "silo/query_engine/filter/expressions/insertion_contains.h"

#include <utility>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/insertion_format_exception.h"

namespace silo::query_engine::filter::expressions {

template <typename SymbolType>
InsertionContains<SymbolType>::InsertionContains(
   std::optional<std::string> sequence_name,
   uint32_t position_idx,
   std::string value
)
    : sequence_name(std::move(sequence_name)),
      position_idx(position_idx),
      value(std::move(value)) {}

template <typename SymbolType>
std::string InsertionContains<SymbolType>::toString() const {
   const std::string symbol_name = std::string(SymbolType::SYMBOL_NAME);
   const std::string sequence_string = sequence_name
                                          ? "The sequence '" + sequence_name.value() + "'"
                                          : "The default " + symbol_name + " sequence ";

   return sequence_string + " has insertion '" + value + "'";
}

template <typename SymbolType>
std::unique_ptr<Expression> InsertionContains<SymbolType>::rewrite(
   const storage::Table& /*table*/,
   AmbiguityMode /*mode*/
) const {
   return std::make_unique<InsertionContains<SymbolType>>(sequence_name, position_idx, value);
}

template <typename SymbolType>
std::unique_ptr<operators::Operator> InsertionContains<SymbolType>::compile(
   const storage::Table& table
) const {
   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, *table.schema);

   const std::map<std::string, storage::column::SequenceColumn<SymbolType>>& sequence_stores =
      table.columns.getColumns<typename SymbolType::Column>();

   const storage::column::SequenceColumn<SymbolType>& sequence_store =
      sequence_stores.at(valid_sequence_name);
   const size_t reference_sequence_size = sequence_store.metadata->reference_sequence.size();
   CHECK_SILO_QUERY(
      position_idx <= reference_sequence_size,
      "the requested insertion position ({}) is larger than the length of the reference sequence "
      "({}) for sequence '{}'",
      position_idx,
      reference_sequence_size,
      valid_sequence_name
   );
   return std::make_unique<operators::BitmapProducer>(
      [&]() {
         try {
            auto search_result = sequence_store.insertion_index.search(position_idx, value);
            return CopyOnWriteBitmap(std::move(*search_result));
         } catch (const silo::storage::InsertionFormatException& exception) {
            throw silo::query_engine::IllegalQueryException(
               "The field 'value' in the InsertionContains expression does not contain a valid "
               "regex "
               "pattern: \"{}\". It must only consist of {} symbols and the regex symbol '.*'. "
               "Also note "
               "that the stop codon * must be escaped correctly with a \\ in amino acid queries.",
               value,
               SymbolType::SYMBOL_NAME_LOWER_CASE
            );
         }
      },
      table.sequence_count
   );
}

template class InsertionContains<Nucleotide>;
template class InsertionContains<AminoAcid>;

}  // namespace silo::query_engine::filter::expressions
