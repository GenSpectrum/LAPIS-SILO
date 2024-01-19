#include "silo/query_engine/operators/union.h"

#include <string>
#include <utility>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Union::Union(std::vector<std::unique_ptr<Operator>>&& children, uint32_t row_count)
    : children(std::move(children)),
      row_count(row_count) {}

Union::~Union() noexcept = default;

std::string Union::toString() const {
   std::string res = "(" + children[0]->toString();
   for (size_t i = 1; i < children.size(); ++i) {
      const auto& child = children[i];
      res += " | " + child->toString();
   }
   res += ")";
   return res;
}

Type Union::type() const {
   return UNION;
}

OperatorResult Union::evaluate() const {
   const uint32_t size_of_children = children.size();
   std::vector<const roaring::Roaring*> union_tmp(size_of_children);
   std::vector<OperatorResult> child_res(size_of_children);
   for (uint32_t i = 0; i < size_of_children; i++) {
      child_res[i] = children[i]->evaluate();
      const roaring::Roaring& const_bitmap = *child_res[i];
      union_tmp[i] = &const_bitmap;
   }
   return OperatorResult(roaring::Roaring::fastunion(union_tmp.size(), union_tmp.data()));
}

std::unique_ptr<Operator> Union::copy() const {
   std::vector<std::unique_ptr<Operator>> children_copy;
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(children_copy),
      [](const auto& child) { return child->copy(); }
   );
   return std::make_unique<Union>(std::move(children_copy), row_count);
}

std::unique_ptr<Operator> Union::negate() const {
   return std::make_unique<Complement>(this->copy(), row_count);
}

}  // namespace silo::query_engine::operators
