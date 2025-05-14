#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {
   void validateOrderByFields(const schema::TableSchema& schema) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override;

  public:
   std::vector<std::string> sequence_names;
   std::vector<std::string> additional_fields;
   explicit FastaAligned(std::vector<std::string>&& sequence_names, std::vector<std::string>&& additional_fields);

   QueryPlan toQueryPlan() override;

   std::vector<schema::ColumnIdentifier> getOutputSchema(
      const silo::schema::TableSchema& table_schema
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
