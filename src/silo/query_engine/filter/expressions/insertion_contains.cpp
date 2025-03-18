#include "silo/query_engine/filter/expressions/insertion_contains.h"

#include <ostream>
#include <regex>
#include <sstream>
#include <utility>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/bitmap_producer.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/union.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/insertion_format_exception.h"
#include "silo/storage/table_partition.h"

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
std::unique_ptr<silo::query_engine::filter::operators::Operator> InsertionContains<SymbolType>::
   compile(
      const Database& database,
      const storage::TablePartition& table_partition,
      Expression::AmbiguityMode /*mode*/
   ) const {
   if (table_partition.columns.getColumns<typename SymbolType::Column>().empty()) {
      return std::make_unique<operators::Empty>(table_partition.sequence_count);
   }

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, database.table->schema);

   const std::map<std::string, storage::column::SequenceColumnPartition<SymbolType>>&
      sequence_stores = table_partition.columns.getColumns<typename SymbolType::Column>();

   const storage::column::SequenceColumnPartition<SymbolType>& sequence_store =
      sequence_stores.at(valid_sequence_name);
   return std::make_unique<operators::BitmapProducer>(
      [&]() {
         try {
            auto search_result = sequence_store.insertion_index.search(position_idx, value);
            return CopyOnWriteBitmap(std::move(*search_result));
         } catch (const silo::storage::InsertionFormatException& exception) {
            throw silo::BadRequest(exception.what());
         }
      },
      table_partition.sequence_count
   );
}

namespace {

template <typename SymbolType>
std::regex buildValidInsertionSearchRegex();
template <>
std::regex buildValidInsertionSearchRegex<AminoAcid>() {
   // Build the following regex pattern: ^([symbols]|\.\*)*$
   std::stringstream regex_pattern_string;
   regex_pattern_string << "^([";
   for (const auto symbol : AminoAcid::SYMBOLS) {
      if (symbol != AminoAcid::Symbol::STOP) {
         regex_pattern_string << AminoAcid::symbolToChar(symbol);
      }
   }
   regex_pattern_string << R"(]|(\\\*)|(\.\*))*$)";
   return std::regex(regex_pattern_string.str());
}

template <>
std::regex buildValidInsertionSearchRegex<Nucleotide>() {
   // Build the following regex pattern: ^([symbols]|\.\*)*$
   std::stringstream regex_pattern_string;
   regex_pattern_string << "^([";
   for (const auto symbol : Nucleotide::SYMBOLS) {
      regex_pattern_string << Nucleotide::symbolToChar(symbol);
   }
   regex_pattern_string << "]|\\.\\*)*$";
   return std::regex(regex_pattern_string.str());
}

template <typename SymbolType>
const std::regex VALID_INSERTION_SEARCH_VALUE_REGEX = buildValidInsertionSearchRegex<SymbolType>();

template <typename SymbolType>
bool validateInsertionSearchValue(const std::string& value) {
   return std::regex_search(value, VALID_INSERTION_SEARCH_VALUE_REGEX<SymbolType>);
}

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionContains<SymbolType>>& filter) {
   CHECK_SILO_QUERY(
      json.contains("position"),
      "The field 'position' is required in an InsertionContains expression"
   );
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in an InsertionContains expression needs to be an unsigned integer"
   );
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an InsertionContains expression"
   );
   CHECK_SILO_QUERY(
      json["value"].is_string() && !json["value"].is_null(),
      "The field 'value' in an InsertionContains expression needs to be a string"
   );
   std::optional<std::string> sequence_name = std::nullopt;
   if (json.contains("sequenceName")) {
      sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position_idx = json["position"].get<uint32_t>();
   const std::string& value = json["value"].get<std::string>();
   CHECK_SILO_QUERY(
      !value.empty(),
      "The field 'value' in an InsertionContains expression must not be an empty string"
   );
   CHECK_SILO_QUERY(
      validateInsertionSearchValue<SymbolType>(value),
      "The field 'value' in the InsertionContains expression does not contain a valid regex "
      "pattern: \"" +
         value + "\". It must only consist of " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
         " symbols and the regex symbol '.*'. Also note that the stop codon * must be escaped "
         "correctly with a \\ in amino acid queries."
   );
   filter = std::make_unique<InsertionContains<SymbolType>>(sequence_name, position_idx, value);
}

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<Nucleotide>>& filter
);

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionContains<AminoAcid>>& filter
);

template class InsertionContains<Nucleotide>;
template class InsertionContains<AminoAcid>;

}  // namespace silo::query_engine::filter::expressions
