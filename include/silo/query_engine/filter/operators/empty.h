#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class Empty : public Operator {
  private:
   uint32_t row_count;

  public:
   explicit Empty(uint32_t row_count);

   ~Empty() noexcept override;

   [[nodiscard]] Type type() const override;

   CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Empty>&& empty);
};

}  // namespace silo::query_engine::filter::operators
