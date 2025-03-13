#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "external/roaring_include_wrapper.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Expression;
}  // namespace silo::query_engine::filter::expressions

namespace silo::query_engine::filter::operators {

class IndexScan : public Operator {
   friend class Operator;

  private:
   std::optional<std::unique_ptr<query_engine::filter::expressions::Expression>> logical_equivalent;
   const roaring::Roaring* bitmap;
   uint32_t row_count;

  public:
   explicit IndexScan(const roaring::Roaring* bitmap, uint32_t row_count);

   explicit IndexScan(
      std::unique_ptr<query_engine::filter::expressions::Expression>&& logical_equivalent,
      const roaring::Roaring* bitmap,
      uint32_t row_count
   );

   ~IndexScan() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<IndexScan>&& index_scan);
};

}  // namespace silo::query_engine::filter::operators
