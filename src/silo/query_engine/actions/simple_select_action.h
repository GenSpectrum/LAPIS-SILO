#pragma once

#include "silo/query_engine/actions/action.h"

namespace silo::query_engine::actions {

class SimpleSelectAction : public Action {
   void validateOrderByFields(const schema::TableSchema& schema) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::actions
