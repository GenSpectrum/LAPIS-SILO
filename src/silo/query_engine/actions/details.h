#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>

#include "silo/query_engine/actions/simple_select_action.h"

namespace silo::query_engine::actions {

class Details : public SimpleSelectAction {
   std::vector<std::string> fields;

  public:
   explicit Details(std::vector<std::string> fields);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

}  // namespace silo::query_engine::actions
