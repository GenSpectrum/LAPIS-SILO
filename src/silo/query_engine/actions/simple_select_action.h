#pragma once

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class SimpleSelectAction : public Action {
   QueryResult execute(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override {
      SILO_PANIC("Legacy execute called on already migrated action. Programming error.");
   }

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const config::QueryOptions& query_options
   ) override;
};

}  // namespace silo::query_engine::actions
