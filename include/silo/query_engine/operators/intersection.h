#ifndef SILO_INTERSECTION_H
#define SILO_INTERSECTION_H

#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
class And;
}

namespace silo::query_engine::operators {

class Intersection : public Operator {
   friend class silo::query_engine::filter_expressions::And;

   std::vector<std::unique_ptr<Operator>> children;
   std::vector<std::unique_ptr<Operator>> negated_children;

  public:
   explicit Intersection(
      std::vector<std::unique_ptr<Operator>>&& children,
      std::vector<std::unique_ptr<Operator>>&& negated_children
   );

   ~Intersection() noexcept override;

   [[nodiscard]] Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_INTERSECTION_H
