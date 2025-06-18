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
#include "silo/query_engine/query_result.h"
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

   // TODO(#758) Legacy sort, should be removed
   void applySort(QueryResult& result) const;
   void applyOffsetAndLimit(QueryResult& result) const;

   virtual void validateOrderByFields(const schema::TableSchema& schema) const = 0;

   // TODO(#758) Legacy query engine implementations, should be removed
   [[nodiscard]] virtual QueryResult execute(
      /// Life time: until query result was delivered (and the lock
      /// inside `FixedDatabase` is released)
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const = 0;

  public:
   Action();
   virtual ~Action() = default;

   QueryPlan toQueryPlan(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
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

   [[nodiscard]] virtual QueryResult executeAndOrder(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const;

  private:
   // If this method is not overloaded, a LegacyResultProducer will be created instead
   virtual arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
      const config::QueryOptions& query_options
   );

  protected:
   arrow::Result<arrow::acero::ExecNode*> addSortNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      const silo::schema::TableSchema& table_schema
   ) const;

   arrow::Result<arrow::acero::ExecNode*> addLimitAndOffsetNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node
   ) const;

   arrow::Result<arrow::acero::ExecNode*> addZstdDecompressNode(
      arrow::acero::ExecPlan* arrow_plan,
      arrow::acero::ExecNode* node,
      const silo::schema::TableSchema& table_schema
   ) const;
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

}  // namespace silo::query_engine::actions
