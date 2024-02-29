#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Threshold : public Operator {
  private:
   std::vector<std::unique_ptr<Operator>> non_negated_children;
   std::vector<std::unique_ptr<Operator>> negated_children;
   uint32_t number_of_matchers;
   bool match_exactly;
   uint32_t row_count;

  public:
   Threshold(
      std::vector<std::unique_ptr<Operator>>&& non_negated_children,
      std::vector<std::unique_ptr<Operator>>&& negated_children,
      uint32_t number_of_matchers,
      bool match_exactly,
      uint32_t row_count
   );

   ~Threshold() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<Threshold>&& threshold);
};

}  // namespace silo::query_engine::operators
