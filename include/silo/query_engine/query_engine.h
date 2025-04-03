#pragma once

#include <string>

#include "silo/database.h"

namespace silo::query_engine {

class QueryResult;

QueryResult executeQuery(const silo::Database& database, const std::string& query);

}  // namespace silo::query_engine
