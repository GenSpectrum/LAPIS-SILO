#include "silo/query_engine/actions/fasta_aligned.h"

#include <iostream>
#include <map>
#include <optional>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/expression.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

FastaAligned::FastaAligned(
   std::vector<std::string>&& sequence_names,
   std::vector<std::string>&& additional_fields
)
    : sequence_names(sequence_names),
      additional_fields(additional_fields) {}

void FastaAligned::validateOrderByFields(const schema::TableSchema& schema) const {
   const std::string& primary_key_field = schema.primary_key.name;
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         field.name == primary_key_field ||
            std::ranges::find(sequence_names, field.name) != std::end(sequence_names),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "The only fields returned by the FastaAligned action are {} and {}",
            field.name,
            fmt::join(sequence_names, ","),
            primary_key_field
         )
      );
   }
}

QueryResult FastaAligned::execute(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> bitmap_filter
) const {
   SILO_PANIC("Legacy execute called on action already migrated action. Programming error.");
}

std::vector<schema::ColumnIdentifier> FastaAligned::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   std::set<schema::ColumnIdentifier> fields;
   for (const auto& sequence_name : sequence_names) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value() && isSequenceColumn(column.value().type),
         fmt::format("The table does not contain the SequenceColumn '{}'", sequence_name)
      );
      fields.emplace(column.value());
   }
   for (const auto& sequence_name : additional_fields) {
      auto column = table_schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value(),
         fmt::format("The table does not contain the Column '{}'", sequence_name)
      );
      fields.emplace(column.value());
   }
   fields.emplace(table_schema.primary_key);
   std::vector<schema::ColumnIdentifier> unique_fields{fields.begin(), fields.end()};
   return unique_fields;
}

QueryPlan FastaAligned::toQueryPlan(
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   std::ostream& output_stream,
   const config::QueryOptions& query_options
) {
   QueryPlan query_plan;
   auto status = toQueryPlanImpl(table, partition_filter_operators, output_stream, query_options)
                    .Value(&query_plan);
   if (!status.ok()) {
      SILO_PANIC("Arrow error: {}", status.ToString());
   };
   return query_plan;
}

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

arrow::Result<QueryPlan> FastaAligned::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::unique_ptr<filter::operators::Operator>>& partition_filter_operators,
   std::ostream& output_stream,
   const config::QueryOptions& query_options
) {
   QueryPlan query_plan;
   arrow::acero::ExecNode* node = query_plan.arrow_plan->EmplaceNode<exec_node::TableScan>(
      query_plan.arrow_plan.get(),
      getOutputSchema(table->schema),
      partition_filter_operators,
      table,
      query_options.materialization_cutoff
   );

   if (auto ordering = getOrdering()) {
      // Create an OrderByNode and put it on top, then replace `node` with the created OrderBy
      ARROW_ASSIGN_OR_RAISE(
         node,
         arrow::acero::MakeExecNode(
            std::string{arrow::acero::OrderByNodeOptions::kName},
            query_plan.arrow_plan.get(),
            {node},
            arrow::acero::OrderByNodeOptions{ordering.value()}
         )
      );
   }
   if (limit.has_value() || offset.has_value()) {
      // Create a FetchNode and put it on top, then replace `node` with the created FetchNode
      arrow::acero::FetchNodeOptions fetch_options(offset.value_or(0), limit.value_or(UINT32_MAX));
      ARROW_ASSIGN_OR_RAISE(
         node,
         arrow::acero::MakeExecNode(
            std::string{arrow::acero::FetchNodeOptions::kName},
            query_plan.arrow_plan.get(),
            {node},
            fetch_options
         )
      );
   }

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
      arrow::acero::MakeExecNode(
         std::string{"project"}, query_plan.arrow_plan.get(), {node}, project_options
      )
   );

   // TODO(#764) make output format configurable
   query_plan.arrow_plan->EmplaceNode<exec_node::NdjsonSink>(
      query_plan.arrow_plan.get(), &output_stream, node
   );

   return query_plan;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") &&
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "FastaAligned action must have the field sequenceName of type string or an array of "
      "strings"
   );
   std::vector<std::string> sequence_names;
   if (json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "FastaAligned action must have the field sequenceName of type string or an array "
            "of strings; while parsing array encountered the element " +
               child.dump() + " which is not of type string"
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }
   std::vector<std::string> additional_fields;
   if (json.contains("additionalFields")) {
      CHECK_SILO_QUERY(
         json["additionalFields"].is_array(),
         "The field `additionalFields` in a FastaAligned action must be an array of strings."
      );
      for (const auto& child : json["additionalFields"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field `additionalFields` in a FastaAligned action must be an array of strings. "
            "Encountered non-string element: " +
               child.dump()
         );
         additional_fields.emplace_back(child.get<std::string>());
      }
   }
   action = std::make_unique<FastaAligned>(std::move(sequence_names), std::move(additional_fields));
}

}  // namespace silo::query_engine::actions
