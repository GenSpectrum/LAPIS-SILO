#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "rhydb/config/runtime_config.h"
#include "rhydb/query_engine/column_assignment.h"
#include "rhydb/query_engine/command/write_command.h"
#include "rhydb/query_engine/saneql/function_registry.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb {
class Database;
}

namespace rhydb::query_engine::command {

/// Builds the command of an `update` statement from its bound `table`, `set` and `where` arguments.
[[nodiscard]] WriteCommandPtr buildUpdate(
   const saneql::BoundArguments& args,
   const saneql::Tables& tables
);

/// `update(<table>, set := {<column> := <value>, ...}, where := <predicate>)`, modelled after SQL's
/// `UPDATE <table> SET <column> = <value>, ... WHERE <predicate>`: assigns each `value`, a scalar
/// expression computed per row, to its column for every row of the table matched by the predicate,
/// or for every row without one. The assignments are validated when the command is built and all
/// values are computed before any is written, so `execute` applies all assignments or none.
class UpdateCommand : public WriteCommand {
   schema::TableName table_name_;
   std::unique_ptr<scalar_expressions::ScalarExpression> predicate_;
   std::vector<ColumnAssignment> assignments_;

  public:
   UpdateCommand(
      schema::TableName table_name,
      std::unique_ptr<scalar_expressions::ScalarExpression> predicate,
      std::vector<ColumnAssignment> assignments
   );

   [[nodiscard]] arrow::Result<std::shared_ptr<arrow::Table>> execute(
      Database& database,
      const config::QueryOptions& query_options,
      std::string_view request_id
   ) override;
};

}  // namespace rhydb::query_engine::command
