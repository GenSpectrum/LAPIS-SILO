#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class PhyloChildFilter : public ScalarExpression {
   std::string column_name;
   std::string internal_node;

  public:
   explicit PhyloChildFilter(std::string column_name, std::string internal_node);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<PhyloChildFilter>(column_name, internal_node);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::PHYLO_CHILD_FILTER;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;

  private:
   [[nodiscard]] std::optional<const roaring::Roaring*> getBitmapForValue(
      const silo::storage::column::StringColumn& phylo_tree_index_column
   ) const;
};

}  // namespace silo::query_engine::scalar_expressions
