#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/simple_select_action.h"

namespace silo::query_engine::actions {

class FastaAligned : public SimpleSelectAction {
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;

  public:
   explicit FastaAligned(
      std::vector<std::string>&& sequence_names,
      std::vector<std::string>&& additional_fields
   );

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
