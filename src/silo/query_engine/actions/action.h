#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_plan.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

struct OrderByField {
   std::string name;
   bool ascending;
};

class Action {
  protected:
   std::vector<OrderByField> order_by_fields;
   std::optional<uint32_t> limit;
   std::optional<uint32_t> offset;
   std::optional<uint32_t> randomize_seed;

   virtual void validateOrderByFields(const schema::TableSchema& schema) const = 0;

  public:
   Action();
   virtual ~Action() = default;

   // Returns the type of this action, which is used for logging the performance by Action type
   virtual std::string_view getType() const = 0;

   QueryPlan toQueryPlan(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   );

   void setOrdering(
      const std::vector<OrderByField>& order_by_fields,
      std::optional<uint32_t> limit,
      std::optional<uint32_t> offset,
      std::optional<uint32_t> randomize_seed
   );

   std::optional<arrow::Ordering> getOrdering() const;

   virtual std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const = 0;

   arrow::Result<arrow::acero::ExecNode*> addOrderingNodes(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      const silo::schema::TableSchema& table_schema
   ) const;

   static std::vector<std::string> deduplicateOrderPreserving(const std::vector<std::string>& fields
   );

  protected:
   arrow::Result<arrow::acero::ExecNode*> addLimitAndOffsetNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node
   ) const;

   arrow::Result<arrow::acero::ExecNode*> addZstdDecompressNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      const silo::schema::TableSchema& table_schema
   ) const;

  private:
   virtual arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const = 0;

   static arrow::Result<arrow::acero::ExecNode*> addSortNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      const std::vector<schema::ColumnIdentifier>& output_fields,
      const arrow::Ordering ordering,
      std::optional<size_t> num_rows_to_produce
   );

   static arrow::Result<arrow::acero::ExecNode*> addRandomizeColumn(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      size_t randomize_seed
   );
};

std::optional<uint32_t> parseLimit(const nlohmann::json& json);

std::optional<uint32_t> parseOffset(const nlohmann::json& json);

std::optional<uint32_t> parseRandomizeSeed(const nlohmann::json& json);

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action);

std::vector<silo::schema::ColumnIdentifier> columnNamesToFields(
   const std::vector<std::string>& column_names,
   const silo::schema::TableSchema& table_schema
);

std::vector<std::string> getNodeValues(
   std::shared_ptr<const storage::Table> table,
   const std::string& column_name,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
);

}  // namespace silo::query_engine::actions
