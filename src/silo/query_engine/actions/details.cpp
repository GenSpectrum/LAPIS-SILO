#include "silo/query_engine/actions/details.h"

#include <utility>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::actions {
Details::Details(std::vector<std::string> fields)
    : fields(std::move(fields)) {}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Details>& action) {
   const std::vector<std::string> fields = json.value("fields", std::vector<std::string>());
   action = std::make_unique<Details>(std::move(fields));
}

}  // namespace silo::query_engine::actions
