#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
class And;
}

namespace silo::query_engine::operators {

class Intersection : public Operator {
   friend class silo::query_engine::filter_expressions::And;

   std::vector<std::unique_ptr<Operator>> children;
   std::vector<std::unique_ptr<Operator>> negated_children;
   uint32_t row_count;

  public:
   explicit Intersection(
      std::vector<std::unique_ptr<Operator>>&& children,
      std::vector<std::unique_ptr<Operator>>&& negated_children,
      uint32_t row_count
   );

   ~Intersection() noexcept override;

   [[nodiscard]] Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators
