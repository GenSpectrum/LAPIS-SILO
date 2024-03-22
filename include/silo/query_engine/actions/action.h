#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace silo::query_engine {
class OperatorResult;
class QueryResult;
}  // namespace silo::query_engine
namespace silo {
class Database;
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
   std::optional<uint32_t> randomize_seed;

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
      std::optional<uint32_t> offset,
      std::optional<uint32_t> randomize_seed
   );

   [[nodiscard]] virtual QueryResult executeAndOrder(
      const Database& database,
      std::vector<OperatorResult> bitmap_filter
   ) const;
};

std::optional<uint32_t> parseLimit(const nlohmann::json& json);

std::optional<uint32_t> parseOffset(const nlohmann::json& json);

std::optional<uint32_t> parseRandomizeSeed(const nlohmann::json& json);

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action);

}  // namespace silo::query_engine::actions
