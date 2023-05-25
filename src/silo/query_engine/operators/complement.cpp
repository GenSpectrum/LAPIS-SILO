#include "silo/query_engine/operators/complement.h"
#include <silo/query_engine/operators/intersection.h>

#include "roaring/roaring.hh"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Complement::Complement(std::unique_ptr<Operator> child, uint32_t sequence_count)
    : child(std::move(child)),
      sequence_count(sequence_count) {}

Complement::~Complement() noexcept = default;

using OperatorVector = std::vector<std::unique_ptr<Operator>>;

std::unique_ptr<Complement> Complement::fromDeMorgan(
   OperatorVector disjunction,
   uint32_t sequence_count
) {
   OperatorVector non_negated_child_operators;
   OperatorVector negated_child_operators;
   for (auto& disjunction_child : disjunction) {
      if (disjunction_child->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(
            std::move(dynamic_cast<operators::Complement*>(disjunction_child.get())->child)
         );
      } else {
         non_negated_child_operators.push_back(std::move(disjunction_child));
      }
   }
   // Now swap negated children and non-negated ones
   auto intersection = std::make_unique<operators::Intersection>(
      std::move(negated_child_operators), std::move(non_negated_child_operators)
   );
   return std::make_unique<Complement>(std::move(intersection), sequence_count);
}

std::string Complement::toString() const {
   return "!" + child->toString();
}

Type Complement::type() const {
   return COMPLEMENT;
}

OperatorResult Complement::evaluate() const {
   auto result = child->evaluate();
   result->flip(0, sequence_count);
   return result;
}

}  // namespace silo::query_engine::operators
