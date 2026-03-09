#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/query.h"
#include "silo/query_engine/query_tree.h"
#include "silo/query_engine/saneql/ast.h"

namespace silo::query_engine::saneql {

std::shared_ptr<query_engine::Query> convertToQuery(const ast::Expression& ast);

QueryNodePtr convertToQueryTree(const ast::Expression& ast);

std::unique_ptr<filter::expressions::Expression> convertToFilter(const ast::Expression& ast);

std::unique_ptr<actions::Action> convertToAction(const ast::MethodCall& method_call);

}  // namespace silo::query_engine::saneql
