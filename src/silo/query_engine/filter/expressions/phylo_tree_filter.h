#pragma once

#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class PhyloChildFilter : public Expression {
   std::string column_name;
   std::string internal_node;

  public:
   explicit PhyloChildFilter(std::string column_name, std::string internal_node);

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::filter::operators::Operator> compile(
      const Database& database,
      const storage::TablePartition& database_partition,
      AmbiguityMode mode
   ) const override;

  private:
   std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::StringColumnPartition& phylo_tree_index_column
   ) const;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloChildFilter>& filter);

}  // namespace silo::query_engine::filter::expressions
