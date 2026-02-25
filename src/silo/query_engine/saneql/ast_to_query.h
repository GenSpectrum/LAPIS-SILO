#pragma once

#include <map>
#include <memory>
#include <string_view>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/query_engine/saneql/ast.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::saneql {

operators::QueryNodePtr convertToQueryTree(
   const ast::Expression& ast,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
);

operators::QueryNodePtr parseAndConvertToQueryTree(
   std::string_view query_string,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
);

operators::QueryNodePtr convertExpression(
   const ast::Expression& ast,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables
);

std::unique_ptr<filter::expressions::Expression> convertToFilter(const ast::Expression& ast);

}  // namespace silo::query_engine::saneql
