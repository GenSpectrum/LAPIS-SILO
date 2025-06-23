#include "silo/query_engine/actions/action.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <map>
#include <memory>
#include <random>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/compute/ordering.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/insertions.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::actions {

Action::Action() = default;

void Action::setOrdering(
   const std::vector<OrderByField>& order_by_fields_,
   std::optional<uint32_t> limit_,
   std::optional<uint32_t> offset_,
   std::optional<uint32_t> randomize_seed_
) {
   order_by_fields = order_by_fields_;
   limit = limit_;
   offset = offset_;
   randomize_seed = randomize_seed_;
}

using arrow::compute::NullPlacement;

std::optional<arrow::Ordering> Action::getOrdering() const {
   using arrow::compute::SortOrder;
   if (order_by_fields.empty()) {
      return std::nullopt;
   }
   auto first_order_by_field = order_by_fields.at(0);

   std::vector<arrow::compute::SortKey> sort_keys;
   for (auto order_by_field : order_by_fields) {
      auto sort_order = order_by_field.ascending ? SortOrder::Ascending : SortOrder::Descending;
      sort_keys.emplace_back(order_by_field.name, sort_order);
   }
   const auto null_placement =
      first_order_by_field.ascending ? NullPlacement::AtStart : NullPlacement::AtEnd;
   return arrow::Ordering{sort_keys, null_placement};
}

// NOLINTNEXTLINE(readability-identifier-naming,misc-use-internal-linkage)
void from_json(const nlohmann::json& json, OrderByField& field) {
   if (json.is_string()) {
      field = {.name = json.get<std::string>(), .ascending = true};
      return;
   }
   CHECK_SILO_QUERY(
      json.is_object() && json.contains("field") && json.contains("order") &&
         json["field"].is_string() && json["order"].is_string(),
      "The orderByField '{}' must be either a string or an object containing the fields "
      "'field':string and 'order':string, where the value of order is 'ascending' or 'descending'",
      json.dump()
   );
   const std::string field_name = json["field"].get<std::string>();
   const std::string order_string = json["order"].get<std::string>();
   CHECK_SILO_QUERY(
      order_string == "ascending" || order_string == "descending",
      "The orderByField '{}' must be either a string or an object containing the fields "
      "'field':string and 'order':string, where the value of order is 'ascending' or 'descending'",
      json.dump()
   );
   field = {.name = field_name, .ascending = order_string == "ascending"};
}

std::optional<uint32_t> parseLimit(const nlohmann::json& json) {
   CHECK_SILO_QUERY(
      !json.contains("limit") ||
         (json["limit"].is_number_unsigned() && json["limit"].get<uint32_t>() > 0),
      "If the action contains a limit, it must be a positive number"
   );
   return json.contains("limit") ? std::optional<uint32_t>(json["limit"].get<uint32_t>())
                                 : std::nullopt;
}

std::optional<uint32_t> parseOffset(const nlohmann::json& json) {
   CHECK_SILO_QUERY(
      !json.contains("offset") || json["offset"].is_number_unsigned(),
      "If the action contains an offset, it must be a non-negative number"
   );
   return json.contains("offset") ? std::optional<uint32_t>(json["offset"].get<uint32_t>())
                                  : std::nullopt;
}

std::optional<uint32_t> parseRandomizeSeed(const nlohmann::json& json) {
   if (!json.contains("randomize")) {
      return std::nullopt;
   }
   if (json["randomize"].is_boolean()) {
      if (json["randomize"].get<bool>()) {
         const uint32_t time_based_seed =
            std::chrono::system_clock::now().time_since_epoch().count();
         return time_based_seed;
      }
      return std::nullopt;
   }
   CHECK_SILO_QUERY(
      json["randomize"].is_object() && json["randomize"].contains("seed") &&
         json["randomize"]["seed"].is_number_unsigned(),
      "If the action contains 'randomize', it must be either a boolean or an object "
      "containing an unsigned 'seed'"
   );
   return json["randomize"]["seed"].get<uint32_t>();
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action) {
   CHECK_SILO_QUERY(json.contains("type"), "The field 'type' is required in any action");
   CHECK_SILO_QUERY(
      json["type"].is_string(),
      "The field 'type' in all actions needs to be a string, but is: {}",
      json["type"].dump()
   );
   const std::string expression_type = json["type"];
   if (expression_type == "Aggregated") {
      action = json.get<std::unique_ptr<Aggregated>>();
   } else if (expression_type == "Mutations") {
      action = json.get<std::unique_ptr<Mutations<Nucleotide>>>();
   } else if (expression_type == "Details") {
      action = json.get<std::unique_ptr<Details>>();
   } else if (expression_type == "AminoAcidMutations") {
      action = json.get<std::unique_ptr<Mutations<AminoAcid>>>();
   } else if (expression_type == "Fasta") {
      action = json.get<std::unique_ptr<Fasta>>();
   } else if (expression_type == "FastaAligned") {
      action = json.get<std::unique_ptr<FastaAligned>>();
   } else if (expression_type == "Insertions") {
      action = json.get<std::unique_ptr<InsertionAggregation<Nucleotide>>>();
   } else if (expression_type == "AminoAcidInsertions") {
      action = json.get<std::unique_ptr<InsertionAggregation<AminoAcid>>>();
   } else {
      throw BadRequest(expression_type + " is not a valid action");
   }

   CHECK_SILO_QUERY(
      !json.contains("orderByFields") || json["orderByFields"].is_array(),
      "orderByFields must be an array."
   );
   auto order_by_fields = json.contains("orderByFields")
                             ? json["orderByFields"].get<std::vector<OrderByField>>()
                             : std::vector<OrderByField>();
   CHECK_SILO_QUERY(
      !json.contains("offset") || json["offset"].is_number_unsigned(),
      "If the action contains an offset, it must be a non-negative number"
   );
   auto limit = parseLimit(json);
   auto offset = parseOffset(json);
   auto randomize_seed = parseRandomizeSeed(json);
   action->setOrdering(order_by_fields, limit, offset, randomize_seed);
}

std::vector<schema::ColumnIdentifier> columnNamesToFields(
   const std::vector<std::string>& column_names,
   const silo::schema::TableSchema& table_schema
) {
   std::vector<schema::ColumnIdentifier> fields;
   for (const auto& column_name : column_names) {
      auto column = table_schema.getColumn(column_name);
      CHECK_SILO_QUERY(column.has_value(), "The table does not contain the field {}", column_name);
      fields.emplace_back(column_name, column.value().type);
   }
   return fields;
}

QueryPlan Action::toQueryPlan(
   std::shared_ptr<const storage::Table> table,
   std::shared_ptr<filter::operators::OperatorVector> partition_filter_operators,
   const config::QueryOptions& query_options
) {
   auto query_plan = toQueryPlanImpl(table, partition_filter_operators, query_options);
   if (!query_plan.status().ok()) {
      SILO_PANIC("Arrow error: {}", query_plan.status().ToString());
   };
   return query_plan.ValueUnsafe();
}

arrow::Result<arrow::acero::ExecNode*> Action::addSortNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const silo::schema::TableSchema& table_schema
) const {
   if (auto ordering = getOrdering()) {
      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> generator;
      // TODO(#800) add optimized sorting when limit is supplied
      //      if (limit.has_value()) {
      //         auto number_of_rows_to_be_sorted = limit.value() + offset.value_or(0);
      //         ARROW_ASSIGN_OR_RAISE(
      //            node,
      //            arrow::acero::MakeExecNode(
      //               "select_k_sink",
      //               arrow_plan,
      //               {node},
      //               arrow::acero::SelectKSinkNodeOptions{
      //                  arrow::SelectKOptions(number_of_rows_to_be_sorted,
      //                  ordering.value().sort_keys()), &generator
      //               }
      //            )
      //         );
      //      } else {
      ARROW_ASSIGN_OR_RAISE(
         node,
         arrow::acero::MakeExecNode(
            "order_by_sink",
            arrow_plan,
            {node},
            arrow::acero::OrderBySinkNodeOptions{arrow::SortOptions{ordering.value()}, &generator}
         )
      );
      //      }
      auto schema = exec_node::columnsToInternalArrowSchema(getOutputSchema(table_schema));
      return arrow::acero::MakeExecNode(
         "source",
         arrow_plan,
         {},
         arrow::acero::SourceNodeOptions{schema, std::move(generator), ordering.value()}
      );
   }
   return node;
}

arrow::Result<arrow::acero::ExecNode*> Action::addLimitAndOffsetNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node
) const {
   if (limit.has_value() || offset.has_value()) {
      arrow::acero::FetchNodeOptions fetch_options(offset.value_or(0), limit.value_or(UINT32_MAX));
      return arrow::acero::MakeExecNode(
         std::string{arrow::acero::FetchNodeOptions::kName}, arrow_plan, {node}, fetch_options
      );
   }
   return node;
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

arrow::Result<arrow::acero::ExecNode*> Action::addZstdDecompressNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const silo::schema::TableSchema& table_schema
) const {
   auto output_fields = getOutputSchema(table_schema);
   bool needs_decompression =
      std::any_of(output_fields.begin(), output_fields.end(), [](const auto& column_identifier) {
         return schema::isSequenceColumn(column_identifier.type);
      });
   if (needs_decompression) {
      std::vector<arrow::compute::Expression> column_expressions;
      std::vector<std::string> column_names;
      for (auto column : getOutputSchema(table_schema)) {
         if (auto reference = storage::column::visit(column.type, ColumnToReferenceSequenceVisitor{}, table_schema, column)) {
            column_expressions.push_back(exec_node::ZstdDecompressExpression::Make(
               arrow::compute::field_ref(arrow::FieldRef{column.name}), reference.value()
            ));
         } else {
            column_expressions.push_back(arrow::compute::field_ref(arrow::FieldRef{column.name}));
         }
         column_names.push_back(column.name);
      }

      arrow::acero::ProjectNodeOptions project_options{column_expressions, column_names};
      return arrow::acero::MakeExecNode(
         std::string{"project"}, arrow_plan, {node}, project_options
      );
   }
   return node;
}

}  // namespace silo::query_engine::actions
