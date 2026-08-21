#pragma once

#include <nlohmann/json_fwd.hpp>

#include "rhydb/query_engine/command/write_command.h"
#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb {
class Database;
}

namespace rhydb::query_engine::command {

/// `<query>.insertInto(<targetTable>)`: runs the wrapped source query and inserts its result rows
/// into the target table. The source query's output columns are matched to the target's columns by
/// name.
class InsertCommand : public WriteCommand {
   operators::QueryNodePtr source_query_;
   schema::TableName target_table_;

  public:
   InsertCommand(operators::QueryNodePtr source_query, schema::TableName target_table);

   [[nodiscard]] nlohmann::json execute(Database& database) override;
};

}  // namespace rhydb::query_engine::command
