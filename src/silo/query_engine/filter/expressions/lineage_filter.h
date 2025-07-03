#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class LineageFilter : public Expression {
   std::string column_name;
   std::optional<std::string> lineage;
   std::optional<silo::common::RecombinantEdgeFollowingMode> sublineage_mode;

  public:
   explicit LineageFilter(
      std::string column_name,
      std::optional<std::string> lineage,
      std::optional<silo::common::RecombinantEdgeFollowingMode> sublineage_mode
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

  private:
   std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::IndexedStringColumnPartition& lineage_column
   ) const;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<LineageFilter>& filter);

}  // namespace silo::query_engine::filter::expressions
