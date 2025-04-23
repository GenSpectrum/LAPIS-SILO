#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"

namespace silo::query_engine {

struct Query {
   std::unique_ptr<filter::expressions::Expression> filter;
   std::unique_ptr<actions::Action> action;

   explicit Query(std::unique_ptr<filter::expressions::Expression> filter,
                  std::unique_ptr<actions::Action> action) : filter(std::move(filter)), action(std::move(action)) {}

   static std::shared_ptr<Query> parseQuery(const std::string& query_string);
};

}  // namespace silo::query_engine
