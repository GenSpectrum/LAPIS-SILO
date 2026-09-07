#pragma once

#include <string>

#include "rhydb/schema/database_schema.h"

namespace rhydb {

template <typename SymbolType>
std::string validateSequenceName(std::string sequence_name, const schema::TableSchema& schema) {
   CHECK_RHYDB_QUERY(
      schema.getColumn(sequence_name).has_value() &&
         schema.getColumn(sequence_name).value().type == SymbolType::COLUMN_TYPE,
      "Database does not contain the {} Sequence with name: '{}'",
      SymbolType::SYMBOL_NAME,
      sequence_name
   );
   return sequence_name;
}

}  // namespace rhydb