#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/simple_select_action.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class Details : public SimpleSelectAction {
   std::vector<std::string> fields;

  public:
   explicit Details(std::vector<std::string> fields);

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action);

}  // namespace silo::query_engine::actions
