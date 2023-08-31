#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
class Or;
}  // namespace silo::query_engine::filter_expressions

namespace silo::query_engine::operators {

class Union : public Operator {
   friend class silo::query_engine::filter_expressions::Or;
   std::vector<std::unique_ptr<Operator>> children;
   uint32_t row_count;

  public:
   explicit Union(std::vector<std::unique_ptr<Operator>>&& children, uint32_t row_count);

   ~Union() noexcept override;

   [[nodiscard]] Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators
