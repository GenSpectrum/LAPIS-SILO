#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

class LineageFilter : public Expression {
   std::string column_name;
   std::optional<std::string> lineage;
   bool include_sublineages;

  public:
   explicit LineageFilter(
      std::string column_name,
      std::optional<std::string> lineage,
      bool include_sublineages
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;

  private:
   std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::IndexedStringColumnPartition& lineage_column
   ) const;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<LineageFilter>& filter);

}  // namespace silo::query_engine::filter_expressions
