#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class Aggregated : public Action {
  public:
   Aggregated(std::vector<std::string> group_by_fields);

   std::vector<schema::ColumnIdentifier> getOutputSchema(const schema::TableSchema& table_schema
   ) const override;

  private:
   std::vector<std::string> group_by_fields;

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const override;

   arrow::Result<QueryPlan> makeAggregateWithoutGrouping(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const;

   arrow::Result<QueryPlan> makeAggregateWithGrouping(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action);

}  // namespace silo::query_engine::actions
