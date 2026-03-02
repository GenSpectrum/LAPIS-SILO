#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class PhyloChildFilter : public Expression {
   std::string column_name;
   std::string internal_node;

  public:
   explicit PhyloChildFilter(std::string column_name, std::string internal_node);

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition
   ) const override;

  private:
   [[nodiscard]] std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::StringColumnPartition& phylo_tree_index_column
   ) const;
};

}  // namespace silo::query_engine::filter::expressions
