#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {
   std::vector<std::string> sequence_names;

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override;

  public:
   explicit FastaAligned(std::vector<std::string>&& sequence_names);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
