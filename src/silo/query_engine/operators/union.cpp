#include "silo/query_engine/operators/union.h"

#include <roaring/roaring.hh>
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Union::Union(std::vector<std::unique_ptr<Operator>>&& children)
    : children(std::move(children)) {}

Union::~Union() noexcept = default;

std::string Union::toString(const Database& database) const {
   std::string res = "(" + children[0]->toString(database);
   for (unsigned i = 1; i < children.size(); ++i) {
      const auto& child = children[i];
      res += " | " + child->toString(database);
   }
   res += ")";
   return res;
}

Type Union::type() const {
   return UNION;
}

OperatorResult Union::evaluate() const {
   const unsigned size_of_children = children.size();
   const roaring::Roaring* union_tmp[size_of_children];  // NOLINT
   OperatorResult child_res[size_of_children];           // NOLINT
   for (unsigned i = 0; i < size_of_children; i++) {
      auto tmp = children[i]->evaluate();
      child_res[i] = tmp;
      union_tmp[i] = tmp.getAsConst();
   }
   auto* result = new roaring::Roaring(roaring::Roaring::fastunion(children.size(), union_tmp));
   for (unsigned i = 0; i < size_of_children; i++) {
      child_res[i].free();
   }
   return OperatorResult{result, nullptr};
}

}  // namespace silo::query_engine::operators
