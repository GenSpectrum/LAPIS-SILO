#pragma once

#include <map>
#include <memory>
#include <string_view>
#include <vector>

#include <arrow/result.h>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

/// Aggregates SymbolType insertions for matching rows.
template <typename SymbolType>
class InsertionsNode final : public QueryNode {
  public:
   static constexpr std::string_view POSITION_FIELD_NAME = "position";
   static constexpr std::string_view INSERTED_SYMBOLS_FIELD_NAME = "insertedSymbols";
   static constexpr std::string_view INSERTION_FIELD_NAME = "insertion";
   static constexpr std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   static constexpr std::string_view COUNT_FIELD_NAME = "count";

   std::shared_ptr<storage::Table> table;
   std::unique_ptr<filter::expressions::Expression> filter;
   std::vector<schema::ColumnIdentifier> sequence_columns;

   InsertionsNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::vector<schema::ColumnIdentifier> sequence_columns
   )
       : table(std::move(table)),
         filter(std::move(filter)),
         sequence_columns(std::move(sequence_columns)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      std::vector<schema::ColumnIdentifier> fields;
      fields.emplace_back(std::string(POSITION_FIELD_NAME), schema::ColumnType::INT32);
      fields.emplace_back(std::string(INSERTED_SYMBOLS_FIELD_NAME), schema::ColumnType::STRING);
      fields.emplace_back(std::string(SEQUENCE_FIELD_NAME), schema::ColumnType::STRING);
      fields.emplace_back(std::string(INSERTION_FIELD_NAME), schema::ColumnType::STRING);
      fields.emplace_back(std::string(COUNT_FIELD_NAME), schema::ColumnType::INT32);
      return fields;
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
