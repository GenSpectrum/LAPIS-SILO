#include "silo/query_engine/query.h"

#include <string>

#include <spdlog/spdlog.h>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/saneql/ast_to_query.h"
#include "silo/query_engine/saneql/parse_exception.h"
#include "silo/query_engine/saneql/parser.h"

using silo::query_engine::actions::Action;
using silo::query_engine::filter::expressions::Expression;

namespace silo::query_engine {

std::shared_ptr<Query> Query::parseQuery(const std::string& query_string) {
   try {
      saneql::Parser parser(query_string);
      auto ast = parser.parse();
      return saneql::convertToQuery(*ast);
   } catch (const saneql::ParseException& ex) {
      throw IllegalQueryException(ex.what());
   }
}

}  // namespace silo::query_engine
