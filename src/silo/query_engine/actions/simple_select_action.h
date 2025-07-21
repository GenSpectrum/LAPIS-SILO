#pragma once

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class SimpleSelectAction : public Action {
   void validateOrderByFields(const schema::TableSchema& schema) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const override;

   // Already log "Select", it will be changed by TODO(#863)
   std::string_view getType() const override { return "Select"; }
};

}  // namespace silo::query_engine::actions
