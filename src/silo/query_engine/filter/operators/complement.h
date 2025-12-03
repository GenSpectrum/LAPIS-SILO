#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class Complement : public Operator {
   friend class Operator;

   std::unique_ptr<Operator> child;
   uint32_t row_count;

  public:
   explicit Complement(std::unique_ptr<Operator> child, uint32_t row_count);

   static std::unique_ptr<Complement> fromDeMorgan(OperatorVector disjunction, uint32_t row_count);

   ~Complement() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Complement>&& complement);
};

}  // namespace silo::query_engine::filter::operators
