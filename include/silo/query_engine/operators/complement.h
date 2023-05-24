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
   friend class silo::query_engine::filter_expressions::And;
   friend class silo::query_engine::filter_expressions::Or;
   friend class silo::query_engine::filter_expressions::Negation;
   friend class silo::query_engine::filter_expressions::NOf;

   std::unique_ptr<Operator> child;
   uint32_t sequence_count;

  public:
   explicit Complement(std::unique_ptr<Operator> child, uint32_t sequence_count);

   static std::unique_ptr<Complement> fromDeMorgan(
      std::vector<std::unique_ptr<Operator>> disjunction,
      uint32_t sequence_count
   );

   ~Complement() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_COMPLEMENT_H
