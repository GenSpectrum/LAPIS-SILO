#include "silo/query_engine/operators/complement.h"

#include <string>
#include <utility>

#include <roaring/roaring.hh>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"

using silo::query_engine::operators::Operator;

namespace silo::query_engine::operators {

Complement::Complement(std::unique_ptr<Operator> child, uint32_t row_count)
    : child(std::move(child)),
      row_count(row_count) {}

Complement::~Complement() noexcept = default;

using OperatorVector = std::vector<std::unique_ptr<Operator>>;

std::unique_ptr<Complement> Complement::fromDeMorgan(
   OperatorVector disjunction,
   uint32_t row_count
) {
   OperatorVector non_negated_child_operators;
   OperatorVector negated_child_operators;
   for (auto& disjunction_child : disjunction) {
      if (disjunction_child->type() == operators::COMPLEMENT) {
         negated_child_operators.emplace_back(Operator::negate(std::move(disjunction_child)));
      } else {
         non_negated_child_operators.push_back(std::move(disjunction_child));
      }
   }
   // Now swap negated children and non-negated ones
   auto intersection = std::make_unique<operators::Intersection>(
      std::move(negated_child_operators), std::move(non_negated_child_operators), row_count
   );
   return std::make_unique<Complement>(std::move(intersection), row_count);
}

std::string Complement::toString() const {
   return "!" + child->toString();
}

Type Complement::type() const {
   return COMPLEMENT;
}

OperatorResult Complement::evaluate() const {
   auto result = child->evaluate();
   result->flip(0, row_count);
   return result;
}

std::unique_ptr<Operator> Complement::negate(std::unique_ptr<Complement>&& complement) {
   return std::move(complement->child);
}

}  // namespace silo::query_engine::operators
