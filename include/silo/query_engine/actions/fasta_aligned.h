#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/actions/simple_select_action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class FastaAligned : public SimpleSelectAction {
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;

  public:
   explicit FastaAligned(
      std::vector<std::string>&& sequence_names,
      std::vector<std::string>&& additional_fields
   );

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
