#ifndef SILO_COMPLEMENT_H
#define SILO_COMPLEMENT_H

#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::filter_expressions {
class And;
class Or;
class Negation;
class NOf;
}  // namespace silo::query_engine::filter_expressions

namespace silo::query_engine::operators {

class Complement : public Operator {
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

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_COMPLEMENT_H
