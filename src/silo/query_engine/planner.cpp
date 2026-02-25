#include "silo/query_engine/planner.h"

#include <stdexcept>
#include <unordered_set>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/count_filter_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/project_node.h"
#include "silo/query_engine/operators/scan_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/unresolved_insertions_node.h"
#include "silo/query_engine/operators/unresolved_most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/unresolved_mutations_node.h"
#include "silo/query_engine/operators/unresolved_phylo_subtree_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"
#include "silo/query_engine/saneql/ast_to_query.h"

namespace silo::query_engine {

namespace {

arrow::Result<QueryPlan> planQueryOrError(
   const silo::query_engine::operators::QueryNode& node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   ARROW_ASSIGN_OR_RAISE(auto partial_query_plan, node.toQueryPlan(tables, query_options));
   return query_engine::QueryPlan::makeQueryPlan(
      partial_query_plan.plan, partial_query_plan.top_node, request_id
   );
}

operators::QueryNodePtr wrapWithDecompressIfNeeded(
   operators::QueryNodePtr node,
   const std::shared_ptr<schema::TableSchema>& table_schema
) {
   std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>
      table_schemas_for_decompression;
   for (const auto& column_identifier : node->getOutputSchema()) {
      if (schema::isSequenceColumn(column_identifier.type)) {
         table_schemas_for_decompression.emplace(column_identifier, table_schema);
      }
   }
   if (table_schemas_for_decompression.empty()) {
      return node;
   }
   return std::make_unique<operators::ZstdDecompressNode>(
      std::move(node), std::move(table_schemas_for_decompression)
   );
}

std::shared_ptr<storage::Table> resolveTable(
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto iter = tables.find(table_name);
   CHECK_SILO_QUERY(iter != tables.end(), "table '{}' not found in database", table_name.getName());
   return iter->second;
}

struct ExtractedScanInfo {
   schema::TableName table_name;
   std::unique_ptr<filter::expressions::Expression> filter;
};

/// Extracts table name and filter from a ScanNode or FilterNode(ScanNode) chain.
std::optional<ExtractedScanInfo> extractScanInfo(operators::QueryNodePtr& node) {
   auto* scan = dynamic_cast<operators::ScanNode*>(node.get());
   if (scan != nullptr) {
      return ExtractedScanInfo{.table_name=scan->table_name, .filter=std::make_unique<filter::expressions::True>()};
   }
   auto* filter = dynamic_cast<operators::FilterNode*>(node.get());
   if (filter != nullptr) {
      auto* inner_scan = dynamic_cast<operators::ScanNode*>(filter->child.get());
      if (inner_scan != nullptr) {
         return ExtractedScanInfo{.table_name=inner_scan->table_name, .filter=std::move(filter->filter)};
      }
   }
   return std::nullopt;
}

template <typename SymbolType>
operators::QueryNodePtr pushdownUnresolvedMutations(
   operators::UnresolvedMutationsNode<SymbolType>* unresolved,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto scan_info = extractScanInfo(unresolved->child);
   CHECK_SILO_QUERY(scan_info.has_value(), "mutations() must be applied to a table scan");

   auto table = resolveTable(scan_info->table_name, tables);

   std::vector<schema::ColumnIdentifier> bound_sequence_columns;
   for (const auto& sequence_name : unresolved->sequence_names) {
      auto column_identifier = table->schema->getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column_identifier.has_value() && column_identifier.value().type == SymbolType::COLUMN_TYPE,
         "The database does not contain the {} sequence '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      );
      bound_sequence_columns.emplace_back(column_identifier.value());
   }
   if (unresolved->sequence_names.empty()) {
      for (const auto& column_identifier :
           table->schema->template getColumnByType<typename SymbolType::Column>()) {
         bound_sequence_columns.emplace_back(column_identifier);
      }
   }

   std::vector<std::string_view> fields_to_use;
   if (unresolved->fields.empty()) {
      fields_to_use = {
         operators::MutationsNode<SymbolType>::MUTATION_FIELD_NAME,
         operators::MutationsNode<SymbolType>::MUTATION_FROM_FIELD_NAME,
         operators::MutationsNode<SymbolType>::MUTATION_TO_FIELD_NAME,
         operators::MutationsNode<SymbolType>::POSITION_FIELD_NAME,
         operators::MutationsNode<SymbolType>::SEQUENCE_FIELD_NAME,
         operators::MutationsNode<SymbolType>::PROPORTION_FIELD_NAME,
         operators::MutationsNode<SymbolType>::COVERAGE_FIELD_NAME,
         operators::MutationsNode<SymbolType>::COUNT_FIELD_NAME
      };
   } else {
      for (const auto& field_str : unresolved->fields) {
         auto it = std::ranges::find(operators::MutationsNode<SymbolType>::VALID_FIELDS, field_str);
         CHECK_SILO_QUERY(
            it != operators::MutationsNode<SymbolType>::VALID_FIELDS.end(),
            "The attribute 'fields' contains an invalid field '{}'. Valid fields are mutation, "
            "mutationFrom, mutationTo, position, sequenceName, proportion, coverage, count.",
            field_str
         );
         fields_to_use.push_back(*it);
      }
   }

   return std::make_unique<operators::MutationsNode<SymbolType>>(
      std::move(table),
      std::move(scan_info->filter),
      std::move(bound_sequence_columns),
      unresolved->min_proportion,
      std::move(fields_to_use)
   );
}

template <typename SymbolType>
operators::QueryNodePtr pushdownUnresolvedInsertions(
   operators::UnresolvedInsertionsNode<SymbolType>* unresolved,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto scan_info = extractScanInfo(unresolved->child);
   CHECK_SILO_QUERY(scan_info.has_value(), "insertions() must be applied to a table scan");

   auto table = resolveTable(scan_info->table_name, tables);

   std::vector<schema::ColumnIdentifier> bound_sequence_columns;
   for (const auto& sequence_name : unresolved->sequence_names) {
      auto column_identifier = table->schema->getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column_identifier.has_value() && column_identifier.value().type == SymbolType::COLUMN_TYPE,
         "The database does not contain the {} sequence '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      );
      bound_sequence_columns.emplace_back(column_identifier.value());
   }
   if (unresolved->sequence_names.empty()) {
      for (const auto& column_identifier :
           table->schema->template getColumnByType<typename SymbolType::Column>()) {
         bound_sequence_columns.emplace_back(column_identifier);
      }
   }

   return std::make_unique<operators::InsertionsNode<SymbolType>>(
      std::move(table), std::move(scan_info->filter), std::move(bound_sequence_columns)
   );
}

operators::QueryNodePtr pushdownUnresolvedPhyloSubtree(
   operators::UnresolvedPhyloSubtreeNode* unresolved,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto scan_info = extractScanInfo(unresolved->child);
   CHECK_SILO_QUERY(scan_info.has_value(), "phyloSubtree() must be applied to a table scan");
   auto table = resolveTable(scan_info->table_name, tables);
   return std::make_unique<operators::PhyloSubtreeNode>(
      std::move(table),
      std::move(scan_info->filter),
      std::move(unresolved->column_name),
      unresolved->print_nodes_not_in_tree,
      unresolved->contract_unary_nodes
   );
}

operators::QueryNodePtr pushdownUnresolvedMostRecentCommonAncestor(
   operators::UnresolvedMostRecentCommonAncestorNode* unresolved,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto scan_info = extractScanInfo(unresolved->child);
   CHECK_SILO_QUERY(
      scan_info.has_value(), "mostRecentCommonAncestor() must be applied to a table scan"
   );
   auto table = resolveTable(scan_info->table_name, tables);
   return std::make_unique<operators::MostRecentCommonAncestorNode>(
      std::move(table),
      std::move(scan_info->filter),
      std::move(unresolved->column_name),
      unresolved->print_nodes_not_in_tree
   );
}

/// Collapses Scan/Filter/Project combinations into a single TableScanNode.
/// Handles: ScanNode, FilterNode(ScanNode), ProjectNode(ScanNode),
///          ProjectNode(FilterNode(ScanNode)), FilterNode(ProjectNode(ScanNode)).
// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr pushdownScanFilterProject(
   operators::QueryNodePtr node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   operators::ProjectNode* project_node = nullptr;
   operators::FilterNode* filter_node = nullptr;
   operators::ScanNode* scan_node = nullptr;
   operators::QueryNode* current = node.get();

   for (int i = 0; i < 3; ++i) {
      if (auto* proj = dynamic_cast<operators::ProjectNode*>(current);
          proj != nullptr && project_node == nullptr) {
         project_node = proj;
         current = proj->child.get();
         continue;
      }
      if (auto* flt = dynamic_cast<operators::FilterNode*>(current);
          flt != nullptr && filter_node == nullptr) {
         filter_node = flt;
         current = flt->child.get();
         continue;
      }
      break;
   }
   scan_node = dynamic_cast<operators::ScanNode*>(current);
   if (scan_node == nullptr) {
      return node;
   }

   auto table = resolveTable(scan_node->table_name, tables);
   std::unique_ptr<filter::expressions::Expression> filter =
      filter_node != nullptr ? std::move(filter_node->filter)
                             : std::make_unique<filter::expressions::True>();

   std::vector<schema::ColumnIdentifier> fields;
   std::unordered_set<std::string> seen_names;
   const auto& source_fields =
      project_node != nullptr ? project_node->fields : scan_node->output_schema;
   for (const auto& field : source_fields) {
      if (seen_names.insert(field.name).second) {
         fields.push_back(field);
      }
   }

   auto table_schema = table->schema;
   operators::QueryNodePtr result = std::make_unique<operators::TableScanNode>(
      std::move(table), std::move(filter), std::move(fields)
   );
   return wrapWithDecompressIfNeeded(std::move(result), table_schema);
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::AggregateNode* node) {
   // Full aggregations (COUNT(*) and only a filter below can be optimized)
   if (node->group_by_fields.empty() && node->aggregates.size() == 1 &&
       node->aggregates[0].function == operators::AggregateFunction::COUNT &&
       dynamic_cast<operators::TableScanNode*>(node->child.get()) != nullptr) {
      auto* table_scan_child = dynamic_cast<operators::TableScanNode*>(node->child.get());
      return std::make_unique<operators::CountFilterNode>(
         std::move(table_scan_child->table), std::move(table_scan_child->filter)
      );
   }
   return std::make_unique<operators::AggregateNode>(
      Planner::optimize(std::move(node->child)),
      std::move(node->group_by_fields),
      std::move(node->aggregates)
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::OrderByNode* node) {
   return std::make_unique<operators::OrderByNode>(
      Planner::optimize(std::move(node->child)), std::move(node->fields), node->randomize_seed
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::FetchNode* node) {
   return std::make_unique<operators::FetchNode>(
      Planner::optimize(std::move(node->child)), node->count, node->offset
   );
}

/// Pushes a ProjectNode below a child OrderByNode/FetchNode when this is safe:
/// - Project(Fetch(X)) -> Fetch(Project(X)): always safe
/// - Project(OrderBy(X)) -> OrderBy(Project(X)): safe iff all sort keys are projected
/// Returns the reordered node, or the original if no reorder was possible.
operators::QueryNodePtr tryReorderProject(operators::QueryNodePtr node) {
   auto* project = dynamic_cast<operators::ProjectNode*>(node.get());
   if (project == nullptr) {
      return node;
   }

   if (auto* fetch = dynamic_cast<operators::FetchNode*>(project->child.get())) {
      auto new_project = std::make_unique<operators::ProjectNode>(
         std::move(fetch->child), std::move(project->fields)
      );
      return std::make_unique<operators::FetchNode>(
         std::move(new_project), fetch->count, fetch->offset
      );
   }

   if (auto* order_by = dynamic_cast<operators::OrderByNode*>(project->child.get())) {
      std::unordered_set<std::string> projected_names;
      for (const auto& field : project->fields) {
         projected_names.insert(field.name);
      }
      for (const auto& order_field : order_by->fields) {
         if (!projected_names.contains(order_field.name)) {
            return node;
         }
      }
      auto new_project = std::make_unique<operators::ProjectNode>(
         std::move(order_by->child), std::move(project->fields)
      );
      return std::make_unique<operators::OrderByNode>(
         std::move(new_project), std::move(order_by->fields), order_by->randomize_seed
      );
   }

   return node;
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr Planner::pushdown(
   operators::QueryNodePtr node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   // Push Project below OrderBy/Fetch when safe, so it can collapse into TableScan.
   if (dynamic_cast<operators::ProjectNode*>(node.get()) != nullptr) {
      operators::QueryNode* before = node.get();
      node = tryReorderProject(std::move(node));
      if (node.get() != before) {
         return pushdown(std::move(node), tables);
      }
   }

   // Try to collapse Scan/Filter/Project combinations into TableScanNode
   if (dynamic_cast<operators::ScanNode*>(node.get()) != nullptr) {
      return pushdownScanFilterProject(std::move(node), tables);
   }
   if (auto* filter_node = dynamic_cast<operators::FilterNode*>(node.get())) {
      if (dynamic_cast<operators::ScanNode*>(filter_node->child.get()) != nullptr ||
          dynamic_cast<operators::ProjectNode*>(filter_node->child.get()) != nullptr) {
         return pushdownScanFilterProject(std::move(node), tables);
      }
   }
   if (auto* project_node = dynamic_cast<operators::ProjectNode*>(node.get())) {
      if (dynamic_cast<operators::ScanNode*>(project_node->child.get()) != nullptr ||
          dynamic_cast<operators::FilterNode*>(project_node->child.get()) != nullptr) {
         return pushdownScanFilterProject(std::move(node), tables);
      }
   }

   // Resolve unresolved mutations/insertions nodes
   if (auto* unresolved =
          dynamic_cast<operators::UnresolvedMutationsNode<Nucleotide>*>(node.get())) {
      return pushdownUnresolvedMutations<Nucleotide>(unresolved, tables);
   }
   if (auto* unresolved =
          dynamic_cast<operators::UnresolvedMutationsNode<AminoAcid>*>(node.get())) {
      return pushdownUnresolvedMutations<AminoAcid>(unresolved, tables);
   }
   if (auto* unresolved =
          dynamic_cast<operators::UnresolvedInsertionsNode<Nucleotide>*>(node.get())) {
      return pushdownUnresolvedInsertions<Nucleotide>(unresolved, tables);
   }
   if (auto* unresolved =
          dynamic_cast<operators::UnresolvedInsertionsNode<AminoAcid>*>(node.get())) {
      return pushdownUnresolvedInsertions<AminoAcid>(unresolved, tables);
   }
   if (auto* unresolved = dynamic_cast<operators::UnresolvedPhyloSubtreeNode*>(node.get())) {
      return pushdownUnresolvedPhyloSubtree(unresolved, tables);
   }
   if (auto* unresolved =
          dynamic_cast<operators::UnresolvedMostRecentCommonAncestorNode*>(node.get())) {
      return pushdownUnresolvedMostRecentCommonAncestor(unresolved, tables);
   }

   // Recurse into nodes with children
   if (auto* aggregate = dynamic_cast<operators::AggregateNode*>(node.get())) {
      aggregate->child = pushdown(std::move(aggregate->child), tables);
      return node;
   }
   if (auto* order_by = dynamic_cast<operators::OrderByNode*>(node.get())) {
      order_by->child = pushdown(std::move(order_by->child), tables);
      return node;
   }
   if (auto* fetch = dynamic_cast<operators::FetchNode*>(node.get())) {
      fetch->child = pushdown(std::move(fetch->child), tables);
      return node;
   }
   if (auto* project = dynamic_cast<operators::ProjectNode*>(node.get())) {
      project->child = pushdown(std::move(project->child), tables);
      return node;
   }
   if (auto* filter_node = dynamic_cast<operators::FilterNode*>(node.get())) {
      filter_node->child = pushdown(std::move(filter_node->child), tables);
      return node;
   }

   return node;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr Planner::optimize(operators::QueryNodePtr node) {
   if (dynamic_cast<operators::AggregateNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::AggregateNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   if (dynamic_cast<operators::OrderByNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::OrderByNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   if (dynamic_cast<operators::FetchNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::FetchNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   return node;
}

QueryPlan Planner::planQuery(
   operators::QueryNodePtr node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   auto pushed_down_tree = pushdown(std::move(node), tables);
   auto optimized_tree = optimize(std::move(pushed_down_tree));
   auto result = planQueryOrError(*optimized_tree, tables, query_options, request_id);
   if (!result.ok()) {
      throw std::runtime_error(
         fmt::format("Error when planning query execution: {}", result.status().ToString())
      );
   }
   return std::move(result.ValueUnsafe());
}

QueryPlan Planner::planSaneqlQuery(
   std::string_view query_string,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   auto query_node = saneql::parseAndConvertToQueryTree(query_string, tables);
   return planQuery(std::move(query_node), tables, query_options, request_id);
}

}  // namespace silo::query_engine
