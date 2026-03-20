#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <nlohmann/json_fwd.hpp>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/actions/order_by_field.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/query_plan.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class Action {
  protected:
   std::vector<OrderByField> order_by_fields;
   std::optional<uint32_t> limit;
   std::optional<uint32_t> offset;
   std::optional<uint32_t> randomize_seed;

  public:
   Action();
   virtual ~Action() = default;

   void setOrdering(
      const std::vector<OrderByField>& order_by_fields,
      std::optional<uint32_t> limit,
      std::optional<uint32_t> offset,
      std::optional<uint32_t> randomize_seed
   );

   [[nodiscard]] const std::vector<OrderByField>& getOrderByFields() const {
      return order_by_fields;
   }

   [[nodiscard]] std::optional<uint32_t> getLimit() const { return limit; }

   [[nodiscard]] std::optional<uint32_t> getOffset() const { return offset; }

   [[nodiscard]] std::optional<uint32_t> getRandomizeSeed() const { return randomize_seed; }
};

std::optional<uint32_t> parseLimit(const nlohmann::json& json);

std::optional<uint32_t> parseOffset(const nlohmann::json& json);

std::optional<uint32_t> parseRandomizeSeed(const nlohmann::json& json);

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Action>& action);

std::vector<silo::schema::ColumnIdentifier> columnNamesToFields(
   const std::vector<std::string>& column_names,
   const silo::schema::TableSchema& table_schema
);

}  // namespace silo::query_engine::actions
