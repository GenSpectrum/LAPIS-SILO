#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;

  public:
   explicit FastaAligned(
      std::vector<std::string>&& sequence_names,
      std::vector<std::string>&& additional_fields
   );

   QueryPlan toQueryPlan(
      std::shared_ptr<const storage::Table> table,
      const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
      std::ostream& output_stream,
      const config::QueryOptions& query_options
   ) override;

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;

  private:
   void validateOrderByFields(const schema::TableSchema& schema) const override;

   [[nodiscard]] QueryResult execute(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
      std::ostream& output_stream,
      const config::QueryOptions& query_options
   );
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
