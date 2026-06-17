#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for insertions action, resolved during pushdown.
template <typename SymbolType>
class UnresolvedInsertionsNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::vector<std::string> sequence_names;

   UnresolvedInsertionsNode(QueryNodePtr child, std::vector<std::string> sequence_names)
       : child(std::move(child)),
         sequence_names(std::move(sequence_names)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      using IN = InsertionsNode<SymbolType>;
      return {
         {.name = std::string(IN::POSITION_FIELD_NAME), .type = schema::ColumnType::INT32},
         {.name = std::string(IN::INSERTED_SYMBOLS_FIELD_NAME), .type = schema::ColumnType::STRING},
         {.name = std::string(IN::SEQUENCE_FIELD_NAME), .type = schema::ColumnType::STRING},
         {.name = std::string(IN::INSERTION_FIELD_NAME), .type = schema::ColumnType::STRING},
         {.name = std::string(IN::COUNT_FIELD_NAME), .type = schema::ColumnType::INT32},
      };
   }

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& /*plan*/,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error("UnresolvedInsertionsNode must be eliminated during pushdown");
   }

   [[nodiscard]] NodeKind kind() const override {
      if constexpr (std::is_same_v<SymbolType, silo::Nucleotide>) {
         return NodeKind::UNRESOLVED_INSERTIONS_NUCLEOTIDE;
      } else {
         return NodeKind::UNRESOLVED_INSERTIONS_AMINO_ACID;
      }
   }

   [[nodiscard]] nlohmann::json toJson() const override {
      return {
         {"type", nodeKindToString(kind())},
         {"sequenceNames", sequence_names},
         {"child", child->toJson()},
      };
   }
};

}  // namespace silo::query_engine::operators
