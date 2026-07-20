#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/storage/column/row_layout.h"

namespace silo::query_engine::scalar_expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class And;
class NOf;
}  // namespace silo::query_engine::scalar_expressions

namespace silo::query_engine::filter::operators {

class Intersection : public Operator {
   friend class silo::query_engine::scalar_expressions::And;
   friend class silo::query_engine::scalar_expressions::NOf;

   OperatorVector children;
   OperatorVector negated_children;
   storage::column::RowLayout row_layout;

  public:
   explicit Intersection(
      OperatorVector&& children,
      OperatorVector&& negated_children,
      storage::column::RowLayout row_layout
   );

   ~Intersection() noexcept override;

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Intersection>&& intersection);
};

}  // namespace silo::query_engine::filter::operators
