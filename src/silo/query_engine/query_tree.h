#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine {

struct Query;
struct QueryNode;
using QueryNodePtr = std::unique_ptr<QueryNode>;

namespace query_tree {

struct TableScan {
   schema::TableName table_name;
};

struct Filter {
   std::unique_ptr<filter::expressions::Expression> predicate;
   QueryNodePtr source;
};

struct Aggregated {
   std::vector<std::string> group_by_fields;
   QueryNodePtr source;
};

struct Details {
   std::vector<std::string> fields;
   QueryNodePtr source;
};

struct NucMutations {
   std::vector<std::string> sequence_names;
   double min_proportion;
   std::vector<std::string> fields;
   QueryNodePtr source;
};

struct AAMutations {
   std::vector<std::string> sequence_names;
   double min_proportion;
   std::vector<std::string> fields;
   QueryNodePtr source;
};

struct NucFasta {
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;
   QueryNodePtr source;
};

struct AlignedFasta {
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;
   QueryNodePtr source;
};

struct NucInsertions {
   std::vector<std::string> sequence_names;
   QueryNodePtr source;
};

struct AAInsertions {
   std::vector<std::string> sequence_names;
   QueryNodePtr source;
};

struct PhyloSubtree {
   std::string column_name;
   bool print_nodes_not_in_tree;
   bool contract_unary_nodes;
   QueryNodePtr source;
};

struct MostRecentCommonAncestor {
   std::string column_name;
   bool print_nodes_not_in_tree;
   QueryNodePtr source;
};

struct OrderBy {
   std::vector<actions::OrderByField> fields;
   QueryNodePtr source;
};

struct Limit {
   std::optional<uint32_t> limit;
   std::optional<uint32_t> offset;
   std::optional<uint32_t> randomize_seed;
   QueryNodePtr source;
};

}  // namespace query_tree

using QueryNodeValue = std::variant<
   query_tree::TableScan,
   query_tree::Filter,
   query_tree::Aggregated,
   query_tree::Details,
   query_tree::NucMutations,
   query_tree::AAMutations,
   query_tree::NucFasta,
   query_tree::AlignedFasta,
   query_tree::NucInsertions,
   query_tree::AAInsertions,
   query_tree::PhyloSubtree,
   query_tree::MostRecentCommonAncestor,
   query_tree::OrderBy,
   query_tree::Limit>;

struct QueryNode {
   QueryNodeValue value;
};

QueryNodePtr makeNode(QueryNodeValue value);

std::shared_ptr<Query> lowerToQuery(QueryNodePtr tree);

}  // namespace silo::query_engine
