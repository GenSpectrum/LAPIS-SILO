#ifndef SILO_AGGREGATED_H
#define SILO_AGGREGATED_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
class Database;

namespace query_engine {
struct OperatorResult;
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::actions {

class Aggregated : public Action {
  private:
   std::vector<std::string> group_by_fields;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   Aggregated(std::vector<std::string> group_by_fields);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Aggregated>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_AGGREGATED_H
