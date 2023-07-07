#ifndef SILO_SELECTION_H
#define SILO_SELECTION_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "silo/common/string.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

template <typename T>
class Selection : public Operator {
  public:
   enum Comparator { EQUALS, LESS, HIGHER, LESS_OR_EQUALS, HIGHER_OR_EQUALS, NOT_EQUALS };

  private:
   const std::vector<T>& column;
   Comparator comparator;
   T value;
   uint32_t row_count;

  public:
   explicit Selection(
      const std::vector<T>& column,
      Comparator comparator,
      T value,
      uint32_t row_count
   );

   ~Selection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_SELECTION_H
