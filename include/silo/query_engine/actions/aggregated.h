#ifndef SILO_AGGREGATED_H
#define SILO_AGGREGATED_H

#include "silo/query_engine/actions/action.h"

#include <optional>

namespace silo::query_engine::actions {

class Aggregated : public Action {
  private:
   std::vector<std::string> group_by_fields;
   std::vector<std::string> order_by_fields;
   std::optional<uint32_t> limit;

  public:
   Aggregated(
      std::vector<std::string> group_by_fields,
      std::vector<std::string> order_by_fields,
      std::optional<uint32_t> limit
   );

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_AGGREGATED_H
