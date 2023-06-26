#ifndef SILO_ACTION_H
#define SILO_ACTION_H

#include <memory>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {
struct OperatorResult;
struct QueryResult;
}  // namespace silo::query_engine
namespace silo {
struct Database;
}
namespace silo::storage {
struct DatabasePartition;
}

namespace silo::query_engine::actions {

class Action {
  public:
   Action();
   virtual ~Action() = default;

   [[nodiscard]] virtual QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const = 0;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_ACTION_H
