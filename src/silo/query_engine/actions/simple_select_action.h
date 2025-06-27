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
};

}  // namespace silo::query_engine::actions
