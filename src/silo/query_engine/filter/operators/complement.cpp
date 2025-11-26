#include "silo/query_engine/filter/operators/complement.h"

#include <string>
#include <utility>

#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/intersection.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

Complement::Complement(std::unique_ptr<Operator> child, uint32_t row_count)
    : child(std::move(child)),
      row_count(row_count) {}

Complement::~Complement() noexcept = default;

using operators::OperatorVector;

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

CopyOnWriteBitmap Complement::evaluate() const {
   EVOBENCH_SCOPE("Complement", "evaluate");
   auto result = child->evaluate();
   result.getMutable().flip(0, row_count);
   return result;
}

std::unique_ptr<Operator> Complement::negate(std::unique_ptr<Complement>&& complement) {
   return std::move(complement->child);
}

}  // namespace silo::query_engine::filter::operators
