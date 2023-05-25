#ifndef SILO_PARSER_H
#define SILO_PARSER_H

#include <rapidjson/pointer.h>
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine {

std::unique_ptr<silo::query_engine::filter_expressions::Expression> parseExpression(
   const Database& database,
   const rapidjson::Value& json_value,
   int exact
);

}  // namespace silo::query_engine

#endif  // SILO_PARSER_H
