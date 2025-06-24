#include "silo/query_engine/filter/expressions/phylo_tree_filter.h"

#include <optional>
#include <utility>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/common/string.h"
#include "silo/database.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/index_scan.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

PhyloChildFilter::PhyloChildFilter(std::string column_name, std::string internal_node)
    : column_name(std::move(column_name)),
      internal_node(std::move(internal_node)) {}

std::string PhyloChildFilter::toString() const {
   return fmt::format("column {} phylo_child_of {}", column_name, internal_node);
};

std::unique_ptr<silo::query_engine::filter::operators::Operator> PhyloChildFilter::compile(
   const Database& /*database*/,
   const storage::TablePartition& database_partition,
   Expression::AmbiguityMode /*mode*/
) const {
   CHECK_SILO_QUERY(
      database_partition.columns.string_columns.contains(column_name),
      "The database does not contain the column '{}'",
      column_name
   );

   SILO_ASSERT(database_partition.columns.string_columns.contains(column_name));
   const auto& string_column = database_partition.columns.string_columns.at(column_name);
   roaring::Roaring internal_node_descendants =
      string_column.getDescendants(internal_node);
   return std::make_unique<operators::IndexScan>(
      &internal_node_descendants, database_partition.sequence_count
   );
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloChildFilter>& filter) {
   CHECK_SILO_QUERY(
      json.contains("column"), "The field 'column' is required in an PhyloChildFilter expression"
   )
   CHECK_SILO_QUERY(
      json["column"].is_string(),
      "The field 'column' in an PhyloChildFilter expression needs to be a string"
   )
   CHECK_SILO_QUERY(
      json.contains("internalNode"),
      "The field 'internalNode' is required in an PhyloChildFilter expression"
   )
   CHECK_SILO_QUERY(
      json["internalNode"].is_string(),
      "The field 'internalNode' in an PhyloChildFilter expression needs to be a string"
   )
   filter = std::make_unique<PhyloChildFilter>(
      json["column"].get<std::string>(), json["internalNode"].get<std::string>()
   );
}

}  // namespace silo::query_engine::filter::expressions
