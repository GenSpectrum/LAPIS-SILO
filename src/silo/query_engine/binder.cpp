#include "silo/query_engine/binder.h"

#include <unordered_set>
#include <vector>

#include <fmt/ranges.h>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/insertions.h"
#include "silo/query_engine/actions/most_recent_common_ancestor.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/actions/phylo_subtree.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/zstd_decompress_node.h"

namespace silo::query_engine {

namespace {

std::vector<std::string> deduplicateOrderPreserving(const std::vector<std::string>& fields) {
   std::vector<std::string> unique_fields;
   std::unordered_set<std::string> seen;

   for (const auto& field : fields) {
      if (seen.insert(field).second) {
         unique_fields.push_back(field);
      }
   }
   return unique_fields;
}

std::shared_ptr<storage::Table> bindTableNameToTable(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const schema::TableName& table_name
) {
   auto iter = tables.find(table_name);
   CHECK_SILO_QUERY(
      iter != tables.end(), "The table {} is not contained in the database", table_name.getName()
   );
   return iter->second;
}

std::vector<schema::ColumnIdentifier> bindFieldsToColumnsByName(
   const std::vector<std::string>& field_names,
   const storage::Table& table
) {
   std::vector<schema::ColumnIdentifier> field_identifiers;
   for (const auto& field_name : field_names) {
      auto col = table.schema->getColumn(field_name);
      CHECK_SILO_QUERY(col.has_value(), "The table does not contain the field {}", field_name);
      field_identifiers.emplace_back(field_name, col.value().type);
   }
   return field_identifiers;
}

operators::QueryNodePtr bindAction(
   actions::FastaAligned* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);

   std::vector<std::string> all_fields;
   all_fields.push_back(table->schema->primary_key.name);
   std::ranges::copy(action->sequence_names, std::back_inserter(all_fields));
   std::ranges::copy(action->additional_fields, std::back_inserter(all_fields));

   all_fields = deduplicateOrderPreserving(all_fields);

   auto bound_fields = bindFieldsToColumnsByName(all_fields, *table);
   return std::make_unique<operators::TableScanNode>(
      std::move(table), std::move(filter_expression), std::move(bound_fields)
   );
}

operators::QueryNodePtr bindAction(
   actions::Fasta* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);

   std::vector<std::string> all_fields;
   all_fields.push_back(table->schema->primary_key.name);
   std::ranges::copy(action->sequence_names, std::back_inserter(all_fields));
   std::ranges::copy(action->additional_fields, std::back_inserter(all_fields));

   all_fields = deduplicateOrderPreserving(all_fields);

   auto bound_fields = bindFieldsToColumnsByName(all_fields, *table);
   return std::make_unique<operators::TableScanNode>(
      std::move(table), std::move(filter_expression), std::move(bound_fields)
   );
}

operators::QueryNodePtr bindAction(
   actions::Details* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);
   std::vector<schema::ColumnIdentifier> bound_fields;
   if (action->fields.empty()) {
      auto all_non_sequence_fields = std::ranges::filter_view(
         table->schema->getColumnIdentifiers(),
         [&](const auto& identifier) { return !schema::isSequenceColumn(identifier.type); }
      );
      bound_fields = {all_non_sequence_fields.begin(), all_non_sequence_fields.end()};
   } else {
      bound_fields = bindFieldsToColumnsByName(deduplicateOrderPreserving(action->fields), *table);
   }
   return std::make_unique<operators::TableScanNode>(
      std::move(table), std::move(filter_expression), std::move(bound_fields)
   );
}

operators::QueryNodePtr bindAction(
   actions::Aggregated* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   std::vector<std::string> group_by_field_names;
   std::ranges::transform(
      action->group_by_fields,
      std::back_inserter(group_by_field_names),
      [](auto& field) { return field.name; }
   );

   group_by_field_names = deduplicateOrderPreserving(group_by_field_names);

   auto table = bindTableNameToTable(tables, table_name);
   auto bound_fields = bindFieldsToColumnsByName(group_by_field_names, *table);
   // fields for table_scan cannot be empty, otherwise, we cannot aggregate without fields
   std::vector<schema::ColumnIdentifier> fields_for_table_scan;
   if (bound_fields.empty()) {
      fields_for_table_scan = {table->schema->primary_key};
   } else {
      fields_for_table_scan = bound_fields;
   }
   auto scan = std::make_unique<operators::TableScanNode>(
      std::move(table), std::move(filter_expression), fields_for_table_scan
   );
   return std::make_unique<operators::AggregateNode>(std::move(scan), std::move(bound_fields));
}

template <typename SymbolType>
operators::QueryNodePtr bindAction(
   actions::Mutations<SymbolType>* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);

   std::vector<schema::ColumnIdentifier> bound_sequence_columns;
   for (const auto& sequence_name : action->sequence_names) {
      auto column_identifier = table->schema->getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column_identifier.has_value() && column_identifier.value().type == SymbolType::COLUMN_TYPE,
         "The database does not contain the {} sequence '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      );
      bound_sequence_columns.emplace_back(column_identifier.value());
   }
   if (action->sequence_names.empty()) {
      for (const auto& column_identifier :
           table->schema->getColumnByType<typename SymbolType::Column>()) {
         bound_sequence_columns.emplace_back(column_identifier);
      }
   }

   std::vector<std::string_view> fields_to_use = action->fields;
   if (fields_to_use.empty()) {
      fields_to_use = {
         actions::Mutations<SymbolType>::MUTATION_FIELD_NAME,
         actions::Mutations<SymbolType>::MUTATION_FROM_FIELD_NAME,
         actions::Mutations<SymbolType>::MUTATION_TO_FIELD_NAME,
         actions::Mutations<SymbolType>::POSITION_FIELD_NAME,
         actions::Mutations<SymbolType>::SEQUENCE_FIELD_NAME,
         actions::Mutations<SymbolType>::PROPORTION_FIELD_NAME,
         actions::Mutations<SymbolType>::COVERAGE_FIELD_NAME,
         actions::Mutations<SymbolType>::COUNT_FIELD_NAME
      };
   }

   return std::make_unique<operators::MutationsNode<SymbolType>>(
      std::move(table),
      std::move(filter_expression),
      bound_sequence_columns,
      action->min_proportion,
      fields_to_use
   );
}

template <typename SymbolType>
operators::QueryNodePtr bindAction(
   actions::InsertionAggregation<SymbolType>* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);

   std::vector<schema::ColumnIdentifier> bound_sequence_columns;
   for (const auto& sequence_name : action->sequence_names) {
      auto column_identifier = table->schema->getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column_identifier.has_value() && column_identifier.value().type == SymbolType::COLUMN_TYPE,
         "The database does not contain the {} sequence '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      );
      bound_sequence_columns.emplace_back(column_identifier.value());
   }
   if (action->sequence_names.empty()) {
      for (const auto& column_identifier :
           table->schema->getColumnByType<typename SymbolType::Column>()) {
         bound_sequence_columns.emplace_back(column_identifier);
      }
   }

   return std::make_unique<operators::InsertionsNode<SymbolType>>(
      std::move(table), std::move(filter_expression), bound_sequence_columns
   );
}

operators::QueryNodePtr bindAction(
   actions::PhyloSubtree* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);
   return std::make_unique<operators::PhyloSubtreeNode>(
      std::move(table),
      std::move(filter_expression),
      action->column_name,
      action->print_nodes_not_in_tree,
      action->contract_unary_nodes
   );
}

operators::QueryNodePtr bindAction(
   actions::MostRecentCommonAncestor* action,
   std::unique_ptr<filter::expressions::Expression> filter_expression,
   const schema::TableName& table_name,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table = bindTableNameToTable(tables, table_name);
   return std::make_unique<operators::MostRecentCommonAncestorNode>(
      std::move(table),
      std::move(filter_expression),
      action->column_name,
      action->print_nodes_not_in_tree
   );
}

operators::QueryNodePtr bindBaseAction(
   ActionQuery action_query,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   if (dynamic_cast<actions::Details*>(action_query.action.get()) != nullptr) {
      auto* specialized_action = dynamic_cast<actions::Details*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::FastaAligned*>(action_query.action.get()) != nullptr) {
      auto* specialized_action = dynamic_cast<actions::FastaAligned*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::Fasta*>(action_query.action.get()) != nullptr) {
      auto* specialized_action = dynamic_cast<actions::Fasta*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::Aggregated*>(action_query.action.get()) != nullptr) {
      auto* specialized_action = dynamic_cast<actions::Aggregated*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::Mutations<Nucleotide>*>(action_query.action.get()) != nullptr) {
      auto* specialized_action =
         dynamic_cast<actions::Mutations<Nucleotide>*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::Mutations<AminoAcid>*>(action_query.action.get()) != nullptr) {
      auto* specialized_action =
         dynamic_cast<actions::Mutations<AminoAcid>*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::InsertionAggregation<Nucleotide>*>(action_query.action.get()) !=
       nullptr) {
      auto* specialized_action =
         dynamic_cast<actions::InsertionAggregation<Nucleotide>*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::InsertionAggregation<AminoAcid>*>(action_query.action.get()) !=
       nullptr) {
      auto* specialized_action =
         dynamic_cast<actions::InsertionAggregation<AminoAcid>*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::PhyloSubtree*>(action_query.action.get()) != nullptr) {
      auto* specialized_action = dynamic_cast<actions::PhyloSubtree*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   if (dynamic_cast<actions::MostRecentCommonAncestor*>(action_query.action.get()) != nullptr) {
      auto* specialized_action =
         dynamic_cast<actions::MostRecentCommonAncestor*>(action_query.action.get());
      return bindAction(
         specialized_action, std::move(action_query.filter), action_query.table_name, tables
      );
   }
   SILO_UNREACHABLE();
}

std::optional<std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>>
getDecompressInfo(
   const std::vector<schema::ColumnIdentifier>& columns,
   const std::shared_ptr<schema::TableSchema>& column_schema
) {
   std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>
      table_schemas_for_decompression;
   for (const auto& column_identifier : columns) {
      if (silo::schema::isSequenceColumn(column_identifier.type)) {
         table_schemas_for_decompression.emplace(column_identifier, column_schema);
      }
   }
   if (table_schemas_for_decompression.empty()) {
      return std::nullopt;
   }
   return table_schemas_for_decompression;
}

}  // namespace

operators::QueryNodePtr Binder::bindQuery(
   ActionQuery action_query,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
) {
   auto table_name = action_query.table_name;
   auto order_by_fields = action_query.action->getOrderByFields();
   auto randomize = action_query.action->getRandomizeSeed();
   auto limit = action_query.action->getLimit();
   auto offset = action_query.action->getOffset();
   auto node = bindBaseAction(std::move(action_query), tables);

   if (!order_by_fields.empty() || randomize) {
      auto field_identifiers = node->getOutputSchema();
      std::vector<std::string> field_names;
      std::ranges::transform(
         field_identifiers,
         std::back_inserter(field_names),
         [](const auto& identifier) { return identifier.name; }
      );

      for (const OrderByField& order_by_field : order_by_fields) {
         CHECK_SILO_QUERY(
            std::ranges::find(field_names, order_by_field.name) != field_names.end(),
            "OrderByField {} is not contained in the result of this operation. "
            "Allowed values are {}.",
            order_by_field.name,
            fmt::join(field_names, ", ")
         );
      }
      // TODO(#800) add optimized sorting when limit is supplied
      node = std::make_unique<operators::OrderByNode>(std::move(node), order_by_fields, randomize);
   }

   if (limit.has_value() || offset.has_value()) {
      node = std::make_unique<operators::FetchNode>(std::move(node), limit, offset);
   }

   auto decompress_info = getDecompressInfo(node->getOutputSchema(), tables.at(table_name)->schema);
   if (decompress_info) {
      node =
         std::make_unique<operators::ZstdDecompressNode>(std::move(node), decompress_info.value());
   }

   return node;
}

}  // namespace silo::query_engine
