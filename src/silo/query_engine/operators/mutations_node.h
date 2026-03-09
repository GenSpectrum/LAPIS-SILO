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

/// Computes SymbolType mutations for matching rows.
template <typename SymbolType>
class MutationsNode final : public QueryNode {
  public:
   constexpr static std::string_view MUTATION_FIELD_NAME = "mutation";
   constexpr static std::string_view MUTATION_FROM_FIELD_NAME = "mutationFrom";
   constexpr static std::string_view MUTATION_TO_FIELD_NAME = "mutationTo";
   constexpr static std::string_view POSITION_FIELD_NAME = "position";
   constexpr static std::string_view SEQUENCE_FIELD_NAME = "sequenceName";
   constexpr static std::string_view PROPORTION_FIELD_NAME = "proportion";
   constexpr static std::string_view COVERAGE_FIELD_NAME = "coverage";
   constexpr static std::string_view COUNT_FIELD_NAME = "count";

   std::shared_ptr<storage::Table> table;
   std::unique_ptr<filter::expressions::Expression> filter;
   std::vector<schema::ColumnIdentifier> sequence_columns;
   double min_proportion;
   std::vector<std::string_view> fields;

   MutationsNode(
      std::shared_ptr<storage::Table> table,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::vector<schema::ColumnIdentifier> sequence_columns,
      double min_proportion,
      std::vector<std::string_view> fields
   )
       : table(std::move(table)),
         filter(std::move(filter)),
         sequence_columns(std::move(sequence_columns)),
         min_proportion(min_proportion),
         fields(std::move(fields)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      using silo::schema::ColumnType;
      std::vector<schema::ColumnIdentifier> output_fields;
      if (std::ranges::find(fields, MUTATION_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(MUTATION_FIELD_NAME), ColumnType::STRING);
      }
      if (std::ranges::find(fields, MUTATION_FROM_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(MUTATION_FROM_FIELD_NAME), ColumnType::STRING);
      }
      if (std::ranges::find(fields, MUTATION_TO_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(MUTATION_TO_FIELD_NAME), ColumnType::STRING);
      }
      if (std::ranges::find(fields, SEQUENCE_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(SEQUENCE_FIELD_NAME), ColumnType::STRING);
      }
      if (std::ranges::find(fields, POSITION_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(POSITION_FIELD_NAME), ColumnType::INT32);
      }
      if (std::ranges::find(fields, PROPORTION_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(PROPORTION_FIELD_NAME), ColumnType::FLOAT);
      }
      if (std::ranges::find(fields, COVERAGE_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(COVERAGE_FIELD_NAME), ColumnType::INT32);
      }
      if (std::ranges::find(fields, COUNT_FIELD_NAME) != fields.end()) {
         output_fields.emplace_back(std::string(COUNT_FIELD_NAME), ColumnType::INT32);
      }
      return output_fields;
   }

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
