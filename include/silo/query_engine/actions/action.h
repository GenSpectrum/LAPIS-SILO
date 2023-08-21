#ifndef SILO_ACTION_H
#define SILO_ACTION_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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

struct OrderByField {
   std::string name;
   bool ascending;
};

class Action {
  protected:
   std::vector<OrderByField> order_by_fields;
   std::optional<uint32_t> limit;
   std::optional<uint32_t> offset;

   void applySort(QueryResult& result) const;
   void applyOffsetAndLimit(QueryResult& result) const;

   virtual void validateOrderByFields(const Database& database) const = 0;

   [[nodiscard]] virtual QueryResult execute(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const = 0;

  public:
   Action();
   virtual ~Action() = default;

   void setOrdering(
      const std::vector<OrderByField>& order_by_fields,
      std::optional<uint32_t> limit,
      std::optional<uint32_t> offset
   );

   [[nodiscard]] virtual QueryResult executeAndOrder(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action);

}  // namespace silo::query_engine::actions

#endif  // SILO_ACTION_H
