#include "silo/query_engine/query_tree.h"

#include <chrono>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/aggregated.h"
#include "silo/query_engine/actions/details.h"
#include "silo/query_engine/actions/fasta.h"
#include "silo/query_engine/actions/fasta_aligned.h"
#include "silo/query_engine/actions/insertions.h"
#include "silo/query_engine/actions/most_recent_common_ancestor.h"
#include "silo/query_engine/actions/mutations.h"
#include "silo/query_engine/actions/phylo_subtree.h"
#include "silo/query_engine/filter/expressions/true.h"
#include "silo/query_engine/query.h"

namespace silo::query_engine {

QueryNodePtr makeNode(QueryNodeValue value) {
   return std::make_unique<QueryNode>(QueryNode{std::move(value)});
}

namespace {

std::string_view resolveMutationFieldName(const std::string& name) {
   static const std::unordered_map<std::string, std::string_view> field_map = {
      {"mutation", "mutation"},
      {"mutationFrom", "mutationFrom"},
      {"mutationTo", "mutationTo"},
      {"position", "position"},
      {"sequenceName", "sequenceName"},
      {"proportion", "proportion"},
      {"coverage", "coverage"},
      {"count", "count"},
   };
   auto it = field_map.find(name);
   if (it != field_map.end()) {
      return it->second;
   }
   return {};
}

std::vector<std::string_view> resolveFields(const std::vector<std::string>& field_names) {
   std::vector<std::string_view> fields;
   for (const auto& f : field_names) {
      auto resolved = resolveMutationFieldName(f);
      if (!resolved.empty()) {
         fields.emplace_back(resolved);
      }
   }
   return fields;
}

struct LoweringState {
   std::vector<actions::OrderByField> order_by_fields;
   std::optional<uint32_t> limit;
   std::optional<uint32_t> offset;
   std::optional<uint32_t> randomize_seed;
   std::unique_ptr<actions::Action> action;
   std::unique_ptr<filter::expressions::Expression> filter;
   schema::TableName table_name = schema::TableName::getDefault();
};

/// Walk outside-in: peel OrderBy, Limit, action, Filter, TableScan
void lowerWalk(QueryNode& node, LoweringState& state) {
   std::visit(
      [&](auto& val) {
         using T = std::decay_t<decltype(val)>;

         if constexpr (std::is_same_v<T, query_tree::OrderBy>) {
            state.order_by_fields = std::move(val.fields);
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::Limit>) {
            state.limit = val.limit;
            state.offset = val.offset;
            state.randomize_seed = val.randomize_seed;
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::Aggregated>) {
            state.action = std::make_unique<actions::Aggregated>(std::move(val.group_by_fields));
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::Details>) {
            state.action = std::make_unique<actions::Details>(std::move(val.fields));
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::NucMutations>) {
            auto fields = resolveFields(val.fields);
            state.action = std::make_unique<actions::Mutations<Nucleotide>>(
               std::move(val.sequence_names), val.min_proportion, std::move(fields)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::AAMutations>) {
            auto fields = resolveFields(val.fields);
            state.action = std::make_unique<actions::Mutations<AminoAcid>>(
               std::move(val.sequence_names), val.min_proportion, std::move(fields)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::NucFasta>) {
            state.action = std::make_unique<actions::Fasta>(
               std::move(val.sequence_names), std::move(val.additional_fields)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::AlignedFasta>) {
            state.action = std::make_unique<actions::FastaAligned>(
               std::move(val.sequence_names), std::move(val.additional_fields)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::NucInsertions>) {
            state.action = std::make_unique<actions::InsertionAggregation<Nucleotide>>(
               std::move(val.sequence_names)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::AAInsertions>) {
            state.action = std::make_unique<actions::InsertionAggregation<AminoAcid>>(
               std::move(val.sequence_names)
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::PhyloSubtree>) {
            state.action = std::make_unique<actions::PhyloSubtree>(
               std::move(val.column_name), val.print_nodes_not_in_tree, val.contract_unary_nodes
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::MostRecentCommonAncestor>) {
            state.action = std::make_unique<actions::MostRecentCommonAncestor>(
               std::move(val.column_name), val.print_nodes_not_in_tree
            );
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::Filter>) {
            state.filter = std::move(val.predicate);
            lowerWalk(*val.source, state);
         } else if constexpr (std::is_same_v<T, query_tree::TableScan>) {
            state.table_name = std::move(val.table_name);
         }
      },
      node.value
   );
}

}  // namespace

std::shared_ptr<Query> lowerToQuery(QueryNodePtr tree) {
   LoweringState state;
   lowerWalk(*tree, state);

   if (!state.action) {
      throw std::runtime_error("Query tree has no action node");
   }

   // Apply ordering params to the action
   state.action->setOrdering(
      state.order_by_fields, state.limit, state.offset, state.randomize_seed
   );

   if (!state.filter) {
      state.filter = std::make_unique<filter::expressions::True>();
   }

   return std::make_shared<Query>(
      std::move(state.table_name), std::move(state.filter), std::move(state.action)
   );
}

}  // namespace silo::query_engine
