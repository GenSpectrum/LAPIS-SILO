#pragma once

#include <memory>
#include <string>

#include "rhydb/query_engine/copy_on_write_bitmap.h"
#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/storage/column/row_layout.h"

namespace rhydb::query_engine::scalar_expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Or;
class NOf;
}  // namespace rhydb::query_engine::scalar_expressions

namespace rhydb::query_engine::filter::operators {

class Union : public Operator {
   friend class rhydb::query_engine::scalar_expressions::Or;
   friend class rhydb::query_engine::scalar_expressions::NOf;

   OperatorVector children;
   storage::column::RowLayout row_layout;

  public:
   explicit Union(OperatorVector&& children, storage::column::RowLayout row_layout);

   ~Union() noexcept override;

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Union>&& union_operator);
};

}  // namespace rhydb::query_engine::filter::operators
