#pragma once

#include <optional>
#include <string>

#include "silo/database.h"

namespace silo {

template <typename SymbolType>
std::string validateSequenceName(std::string sequence_name, const schema::TableSchema& schema) {
   CHECK_SILO_QUERY(
      schema.getColumn(sequence_name).has_value() &&
         schema.getColumn(sequence_name).value().type == SymbolType::COLUMN_TYPE,
      fmt::format(
         "Database does not contain the {} Sequence with name: '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      )
   );
   return sequence_name;
}

template <typename SymbolType>
std::string validateSequenceNameOrGetDefault(
   std::optional<std::string> sequence_name,
   const schema::TableSchema& schema
) {
   if (sequence_name.has_value()) {
      return validateSequenceName<SymbolType>(sequence_name.value(), schema);
   }

   auto default_sequence = schema.getDefaultSequenceName<SymbolType>();
   CHECK_SILO_QUERY(
      default_sequence.has_value(),
      "The database has no default " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
         " sequence name"
   );
   return default_sequence.value().name;
}

}  // namespace silo