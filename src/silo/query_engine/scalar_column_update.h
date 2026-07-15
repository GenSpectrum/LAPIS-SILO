#pragma once

#include <string>

#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"

namespace silo::storage {
class ColumnGroup;
}

namespace silo::query_engine {

/// Parses `value` as a SaneQL scalar literal (using the same lexer/parser as queries) and assigns
/// it to every row in `row_ids` of the column identified by `column` within `columns`. The literal
/// must match the column's type, e.g. `3`, `3.14`, `true`, or `'2021-03-15'::date`; the literal
/// `null` clears the matched rows. Only scalar value columns (INT32, FLOAT, DATE32, BOOL) can be
/// updated; other column types raise an IllegalQueryException.
void assignScalarLiteralToColumn(
   storage::ColumnGroup& columns,
   const schema::ColumnIdentifier& column,
   const std::string& value,
   const roaring::Roaring& row_ids
);

}  // namespace silo::query_engine
