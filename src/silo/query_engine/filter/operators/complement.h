#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class Complement : public Operator {
   friend class Operator;

   std::unique_ptr<Operator> child;
   uint32_t row_count;

  public:
   explicit Complement(std::unique_ptr<Operator> child, uint32_t row_count);

   static std::unique_ptr<Complement> fromDeMorgan(
      std::vector<std::unique_ptr<Operator>> disjunction,
      uint32_t row_count
   );

   ~Complement() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Complement>&& complement);
};

}  // namespace silo::query_engine::filter::operators
