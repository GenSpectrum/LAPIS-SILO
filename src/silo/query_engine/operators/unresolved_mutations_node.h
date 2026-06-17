#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for mutations action, resolved during pushdown.
template <typename SymbolType>
class UnresolvedMutationsNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<std::string> sequence_names;
   double min_proportion;
   std::vector<std::string> fields;

   UnresolvedMutationsNode(
      QueryNodePtr child,
      std::vector<std::string> sequence_names,
      double min_proportion,
      std::vector<std::string> fields
   )
       : child(std::move(child)),
         sequence_names(std::move(sequence_names)),
         min_proportion(min_proportion),
         fields(std::move(fields)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      using silo::schema::ColumnType;
      using MN = MutationsNode<SymbolType>;
      const bool include_all = fields.empty();
      auto has = [&](std::string_view name) -> bool {
         return include_all || std::ranges::find(fields, name) != fields.end();
      };
      std::vector<schema::ColumnIdentifier> output_fields;
      if (has(MN::MUTATION_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::MUTATION_FIELD_NAME), ColumnType::STRING);
      }
      if (has(MN::MUTATION_FROM_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::MUTATION_FROM_FIELD_NAME), ColumnType::STRING);
      }
      if (has(MN::MUTATION_TO_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::MUTATION_TO_FIELD_NAME), ColumnType::STRING);
      }
      if (has(MN::SEQUENCE_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::SEQUENCE_FIELD_NAME), ColumnType::STRING);
      }
      if (has(MN::POSITION_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::POSITION_FIELD_NAME), ColumnType::INT32);
      }
      if (has(MN::PROPORTION_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::PROPORTION_FIELD_NAME), ColumnType::FLOAT);
      }
      if (has(MN::COVERAGE_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::COVERAGE_FIELD_NAME), ColumnType::INT32);
      }
      if (has(MN::COUNT_FIELD_NAME)) {
         output_fields.emplace_back(std::string(MN::COUNT_FIELD_NAME), ColumnType::INT32);
      }
      return output_fields;
   }

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& /*plan*/,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedMutationsNode must be eliminated during pushdown");
   }

   [[nodiscard]] NodeKind kind() const override {
      if constexpr (std::is_same_v<SymbolType, silo::Nucleotide>) {
         return NodeKind::UNRESOLVED_MUTATIONS_NUCLEOTIDE;
      } else {
         return NodeKind::UNRESOLVED_MUTATIONS_AMINO_ACID;
      }
   }

   [[nodiscard]] nlohmann::json toJson() const override {
      return {
         {"type", nodeKindToString(kind())},
         {"sequenceNames", sequence_names},
         {"minProportion", min_proportion},
         {"fields", fields},
         {"child", child->toJson()},
      };
   }
};

}  // namespace silo::query_engine::operators
