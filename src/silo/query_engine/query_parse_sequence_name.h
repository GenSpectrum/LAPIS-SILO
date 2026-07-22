#pragma once

#include <string>

#include "silo/schema/database_schema.h"

namespace silo {

template <typename SymbolType>
std::string validateSequenceName(std::string sequence_name, const schema::TableSchema& schema) {
   CHECK_SILO_QUERY(
      schema.getColumn(sequence_name).has_value() &&
         schema.getColumn(sequence_name).value().type == SymbolType::COLUMN_TYPE,
      "Database does not contain the {} Sequence with name: '{}'",
      SymbolType::SYMBOL_NAME,
      sequence_name
   );
   return sequence_name;
}

}  // namespace silo