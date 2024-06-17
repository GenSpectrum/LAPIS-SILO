#pragma once

#include <optional>
#include <string>

#include "silo/database.h"

namespace silo {

template <typename SymbolType>
std::string validateSequenceName(std::string sequence_name, const silo::Database& database) {
   CHECK_SILO_QUERY(
      database.getSequenceStores<SymbolType>().contains(sequence_name),
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
   const silo::Database& database
) {
   if (sequence_name.has_value()) {
      return validateSequenceName<SymbolType>(sequence_name.value(), database);
   }

   CHECK_SILO_QUERY(
      database.getDefaultSequenceName<SymbolType>().has_value(),
      "The database has no default " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
         " sequence name"
   );

   const auto default_sequence_name = database.getDefaultSequenceName<SymbolType>().value();
   return validateSequenceName<SymbolType>(default_sequence_name, database);
}

}  // namespace silo