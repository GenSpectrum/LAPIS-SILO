#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/schema/database_schema.h"

namespace silo::query_engine {

struct OrderByField {
   schema::ColumnIdentifier field;
   bool ascending;
};

}  // namespace silo::query_engine
