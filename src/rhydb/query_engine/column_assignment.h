#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include "rhydb/config/runtime_config.h"
#include "rhydb/query_engine/saneql/ast.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::storage {
class Table;
}

namespace rhydb::query_engine {

/// The assignment of a value, computed per row, to a column of a table, e.g. `age := 42` or
/// `division := region`.
struct ColumnAssignment {
   schema::ColumnIdentifier column;
   std::unique_ptr<scalar_expressions::ScalarExpression> value;
};

/// Converts `value`, a SaneQL scalar expression over the columns of `table` as queries see them
/// (e.g. `map` assignments), or the literal `null`, into an assignment to the column identified by
/// `column` within `table`. Throws an IllegalQueryException if the column cannot be updated
/// (sequence columns, and string columns backed by a phylogenetic tree), or if the value's type
/// does not fit the column. Integers can be assigned to int32, int64 and float columns; an int64
/// value that does not fit an int32 column is reported when the assignment is applied.
[[nodiscard]] ColumnAssignment makeColumnAssignment(
   const std::shared_ptr<storage::Table>& table,
   const schema::ColumnIdentifier& column,
   const saneql::ast::Expression& value,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
);

/// Computes the value of every assignment for each row of `table` matched by `predicate`, a filter
/// over the table's columns, and assigns it to the row. Returns the number of updated rows. All
/// values are computed with a query over the table as it is before the update, and before any is
/// written, so an assignment never sees another one's result and a failing computation leaves the
/// table unchanged.
size_t applyColumnAssignments(
   const std::shared_ptr<storage::Table>& table,
   const std::vector<ColumnAssignment>& assignments,
   const scalar_expressions::ScalarExpression& predicate,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
);

}  // namespace rhydb::query_engine
