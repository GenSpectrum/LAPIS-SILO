#include "silo/query_engine/filter_expressions/insertion_contains.h"

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
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/bitmap_producer.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/union.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_parse_sequence_name.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/insertion_index.h"
#include "silo/storage/sequence_store.h"

namespace silo::query_engine::filter_expressions {

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
std::unique_ptr<silo::query_engine::operators::Operator> InsertionContains<SymbolType>::compile(
   const silo::Database& database,
   const silo::DatabasePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   if (database_partition.getSequenceStores<SymbolType>().empty()) {
      return std::make_unique<operators::Empty>(database_partition.sequence_count);
   }

   const auto valid_sequence_name =
      validateSequenceNameOrGetDefault<SymbolType>(sequence_name, database);

   const std::map<std::string, SequenceStorePartition<SymbolType>&>& sequence_stores =
      database_partition.getSequenceStores<SymbolType>();

   const SequenceStorePartition<SymbolType>& sequence_store =
      sequence_stores.at(valid_sequence_name);
   return std::make_unique<operators::BitmapProducer>(
      [&]() {
         auto search_result = sequence_store.insertion_index.search(position_idx, value);
         return OperatorResult(std::move(*search_result));
      },
      database_partition.sequence_count
   );
}

namespace {

template <typename SymbolType>
std::regex buildValidInsertionSearchRegex() {
   // Build the following regex pattern: ^([symbols]|\.\*)*$
   std::stringstream regex_pattern_string;
   regex_pattern_string << "^([";
   for (const auto symbol : SymbolType::SYMBOLS) {
      regex_pattern_string << SymbolType::symbolToChar(symbol);
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
   )
   CHECK_SILO_QUERY(
      json["position"].is_number_unsigned(),
      "The field 'position' in an InsertionContains expression needs to be an unsigned integer"
   )
   CHECK_SILO_QUERY(
      json.contains("value"), "The field 'value' is required in an InsertionContains expression"
   )
   CHECK_SILO_QUERY(
      json["value"].is_string() && !json["value"].is_null(),
      "The field 'value' in an InsertionContains expression needs to be a string"
   )
   std::optional<std::string> sequence_name = std::nullopt;
   if (json.contains("sequenceName")) {
      sequence_name = json["sequenceName"].get<std::string>();
   }
   const uint32_t position_idx = json["position"].get<uint32_t>();
   const std::string& value = json["value"].get<std::string>();
   CHECK_SILO_QUERY(
      !value.empty(),
      "The field 'value' in an InsertionContains expression must not be an empty string"
   )
   CHECK_SILO_QUERY(
      validateInsertionSearchValue<SymbolType>(value),
      "The field 'value' in the InsertionContains expression does not contain a valid regex "
      "pattern: \"" +
         value + "\". It must only consist of " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
         " symbols and the regex symbol '.*'."
   )
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

}  // namespace silo::query_engine::filter_expressions