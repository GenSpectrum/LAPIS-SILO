#include "silo/query_engine/actions/tree_action.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column_group.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {
using silo::schema::ColumnType;

TreeAction::TreeAction(std::string column_name, bool print_nodes_not_in_tree)
    : column_name(std::move(column_name)),
      print_nodes_not_in_tree(print_nodes_not_in_tree) {}

}  // namespace silo::query_engine::actions
