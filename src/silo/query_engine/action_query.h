#pragma once

#include <memory>
#include <string>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"

namespace silo::query_engine {

struct ActionQuery {
   schema::TableName table_name;
   std::unique_ptr<filter::expressions::Expression> filter;
   std::unique_ptr<actions::Action> action;

   explicit ActionQuery(
      schema::TableName table_name,
      std::unique_ptr<filter::expressions::Expression> filter,
      std::unique_ptr<actions::Action> action
   )
       : table_name(std::move(table_name)),
         filter(std::move(filter)),
         action(std::move(action)) {}

   explicit ActionQuery(
      std::unique_ptr<filter::expressions::Expression> filter,
      std::unique_ptr<actions::Action> action
   )
       : ActionQuery(schema::TableName::getDefault(), std::move(filter), std::move(action)) {}

   static ActionQuery parseQuery(const std::string& query_string);
};

}  // namespace silo::query_engine
