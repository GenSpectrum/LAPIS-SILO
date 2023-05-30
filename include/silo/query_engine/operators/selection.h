#ifndef SILO_SELECTION_H
#define SILO_SELECTION_H

#include <vector>

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

  public:
   explicit Selection(const std::vector<T>& column, Comparator comparator, T value);

   ~Selection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual void negate();
};

}  // namespace silo::query_engine::operators

#endif  // SILO_SELECTION_H
