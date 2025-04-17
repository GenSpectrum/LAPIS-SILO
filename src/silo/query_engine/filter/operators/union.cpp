#include "silo/query_engine/filter/operators/union.h"

#include <string>
#include <utility>
#include <vector>

#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

Union::Union(OperatorVector&& children, uint32_t row_count)
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

CopyOnWriteBitmap Union::evaluate() const {
   EVOBENCH_SCOPE("Union", "evaluate");
   const uint32_t size_of_children = children.size();
   std::vector<const roaring::Roaring*> union_tmp(size_of_children);
   std::vector<CopyOnWriteBitmap> child_res(size_of_children);
   for (uint32_t i = 0; i < size_of_children; i++) {
      child_res[i] = children[i]->evaluate();
      const roaring::Roaring& const_bitmap = *child_res[i];
      union_tmp[i] = &const_bitmap;
   }
   return CopyOnWriteBitmap(roaring::Roaring::fastunion(union_tmp.size(), union_tmp.data()));
}

std::unique_ptr<Operator> Union::negate(std::unique_ptr<Union>&& union_operator) {
   return std::make_unique<Complement>(std::move(union_operator), union_operator->row_count);
}

}  // namespace silo::query_engine::filter::operators
