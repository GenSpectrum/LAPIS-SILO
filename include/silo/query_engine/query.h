#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine {

struct Query {
   std::unique_ptr<filter_expressions::Expression> filter;
   std::unique_ptr<actions::Action> action;

   explicit Query(const std::string& query_string);
};

}  // namespace silo::query_engine
