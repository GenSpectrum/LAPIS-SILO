#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Or;
class NOf;
}  // namespace silo::query_engine::filter::expressions

namespace silo::query_engine::filter::operators {

class Union : public Operator {
   friend class silo::query_engine::filter::expressions::Or;
   friend class silo::query_engine::filter::expressions::NOf;

   OperatorVector children;
   uint32_t row_count;

  public:
   explicit Union(OperatorVector&& children, uint32_t row_count);

   ~Union() noexcept override;

   virtual std::string toString() const override;

   [[nodiscard]] Type type() const override;

   virtual CopyOnWriteBitmap evaluate() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Union>&& union_operator);
};

}  // namespace silo::query_engine::filter::operators
