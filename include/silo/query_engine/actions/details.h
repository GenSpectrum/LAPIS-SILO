#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/query_result.h"

namespace silo {
class Database;
namespace query_engine {
class OperatorResult;
}  // namespace query_engine
}  // namespace silo

namespace silo::query_engine::actions {

class Details : public Action {
   std::vector<std::string> fields;

   void validateOrderByFields(const Database& database) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;

  public:
   Details(const Details&) = default;
   Details(Details&&) = default;
   Details& operator=(const Details&) = default;
   Details& operator=(Details&&) = default;
   explicit Details(std::vector<std::string> fields);

   [[nodiscard]] QueryResult executeAndOrder(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action);

}  // namespace silo::query_engine::actions
