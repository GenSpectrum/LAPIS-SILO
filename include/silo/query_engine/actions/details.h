#ifndef SILO_DETAILS_H
#define SILO_DETAILS_H

#include <memory>
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

class Details : public Action {
   std::vector<std::string> fields;

   QueryResult execute(const Database& database, std::vector<OperatorResult> bitmap_filter)
      const override;

  public:
   explicit Details(std::vector<std::string> fields);
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_DETAILS_H
