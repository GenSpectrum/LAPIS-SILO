#include "silo/query_engine/actions/simple_select_action.h"

#include <ranges>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/expression.h>
#include <fmt/ranges.h>

#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::actions {

namespace {

using silo::schema::ColumnIdentifier;
using silo::schema::TableSchema;
using silo::storage::column::Column;
using silo::storage::column::SequenceColumnPartition;
using silo::storage::column::ZstdCompressedStringColumnPartition;

class ColumnToReferenceSequenceVisitor {
  public:
   template <Column ColumnType>
   std::optional<std::string> operator()(
      const TableSchema& table_schema,
      const ColumnIdentifier& column_identifier
   ) {
      return std::nullopt;
   }
};

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<Nucleotide>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto metadata =
      table_schema.getColumnMetadata<SequenceColumnPartition<Nucleotide>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), Nucleotide::symbolToChar
   );
   return reference;
}

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<AminoAcid>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto metadata =
      table_schema.getColumnMetadata<SequenceColumnPartition<AminoAcid>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), AminoAcid::symbolToChar
   );
   return reference;
}

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<ZstdCompressedStringColumnPartition>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto metadata =
      table_schema.getColumnMetadata<ZstdCompressedStringColumnPartition>(column_identifier.name)
         .value();
   return metadata->dictionary_string;
}

}  // namespace

void SimpleSelectAction::validateOrderByFields(const schema::TableSchema& schema) const {
   auto output_schema = getOutputSchema(schema);
   auto output_schema_fields = output_schema |
                               std::views::transform([](const auto& x) { return x.name; }) |
                               std::views::common;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::find(output_schema_fields, field.name) != std::end(output_schema_fields),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by this action are {}",
            field.name,
            fmt::join(output_schema_fields, ", ")
         )
      )
   }
}

arrow::Result<QueryPlan> SimpleSelectAction::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   const config::QueryOptions& query_options
) {
   validateOrderByFields(table->schema);
   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());
   arrow::acero::ExecNode* node = arrow_plan->EmplaceNode<exec_node::TableScan>(
      arrow_plan.get(),
      getOutputSchema(table->schema),
      partition_filter_operators,
      table,
      query_options.materialization_cutoff
   );

   ARROW_ASSIGN_OR_RAISE(node, addSortNode(arrow_plan.get(), node));

   ARROW_ASSIGN_OR_RAISE(node, addLimitAndOffsetNode(arrow_plan.get(), node));

   std::vector<arrow::compute::Expression> column_expressions;
   std::vector<std::string> column_names;
   for (auto column : getOutputSchema(table->schema)) {
      if (auto reference = storage::column::visit(column.type, ColumnToReferenceSequenceVisitor{}, table->schema, column)) {
         column_expressions.push_back(exec_node::ZstdDecompressExpression::Make(
            arrow::compute::field_ref(arrow::FieldRef{column.name}), reference.value()
         ));
      } else {
         column_expressions.push_back(arrow::compute::field_ref(arrow::FieldRef{column.name}));
      }
      column_names.push_back(column.name);
   }

   arrow::acero::ProjectNodeOptions project_options{column_expressions, column_names};
   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode(std::string{"project"}, arrow_plan.get(), {node}, project_options)
   );

   return QueryPlan::makeQueryPlan(arrow_plan, node);
}

}  // namespace silo::query_engine::actions