#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Full : public Operator {
   uint32_t row_count;

  public:
   explicit Full(uint32_t row_count);

   ~Full() noexcept override;

   [[nodiscard]] Type type() const override;

   CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Full>&& full_operator);
};

}  // namespace silo::query_engine::operators
