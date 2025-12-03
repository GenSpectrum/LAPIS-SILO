#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class Threshold : public Operator {
  private:
   OperatorVector non_negated_children;
   OperatorVector negated_children;
   uint32_t number_of_matchers;
   bool match_exactly;
   uint32_t row_count;

  public:
   Threshold(
      OperatorVector&& non_negated_children,
      OperatorVector&& negated_children,
      uint32_t number_of_matchers,
      bool match_exactly,
      uint32_t row_count
   );

   ~Threshold() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Threshold>&& threshold);
};

}  // namespace silo::query_engine::filter::operators
