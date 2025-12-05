#pragma once

#include <memory>
#include <string>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"

namespace silo::query_engine {

using filter::expressions::Expression;

struct Query {
   schema::TableName table_name;
   std::unique_ptr<Expression> filter;
   std::unique_ptr<actions::Action> action;

   explicit Query(
      schema::TableName table_name,
      std::unique_ptr<Expression> filter,
      std::unique_ptr<actions::Action> action
   )
       : table_name(std::move(table_name)),
         filter(std::move(filter)),
         action(std::move(action)) {}

   explicit Query(std::unique_ptr<Expression> filter, std::unique_ptr<actions::Action> action)
       : Query(schema::TableName::getDefault(), std::move(filter), std::move(action)) {}

   static std::shared_ptr<Query> parseQuery(const std::string& query_string);
};

}  // namespace silo::query_engine
