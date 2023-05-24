#include "silo/query_engine/operators/intersection.h"

#include <spdlog/spdlog.h>
#include <roaring/roaring.hh>
#include <vector>

#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Intersection::Intersection(
   std::vector<std::unique_ptr<Operator>>&& children,
   std::vector<std::unique_ptr<Operator>>&& negated_children
)
    : children(std::move(children)),
      negated_children(std::move(negated_children)) {
   if (this->children.empty()) {
      SPDLOG_ERROR(
         "Compilation bug: Intersection without non-negated children is not allowed. "
         "Should be compiled as a union."
      );
      this->children.push_back(std::make_unique<operators::Empty>());
   }
   if (this->children.size() + this->negated_children.size() < 2) {
      SPDLOG_ERROR("Compilation bug: Intersection needs at least two children.");
      this->children.push_back(std::make_unique<operators::Empty>());
   }
}

Intersection::~Intersection() noexcept = default;

std::string Intersection::toString() const {
   std::string res = "(" + this->children[0]->toString();
   for (const auto& child : children) {
      res += " & " + child->toString();
   }
   for (const auto& child : negated_children) {
      res += " &! " + child->toString();
   }
   res += ")";
   return res;
}

Type Intersection::type() const {
   return INTERSECTION;
}

roaring::Roaring* intersectTwo(OperatorResult first, OperatorResult second) {
   roaring::Roaring* result;
   if (first.isMutable()) {
      result = first.getMutable();
      *result &= *second.getConst();
      second.free();
   } else if (second.isMutable()) {
      result = second.getMutable();
      *result &= *first.getConst();
   } else {
      const auto bitmap = *first.getConst() & *second.getConst();
      result = new roaring::Roaring(bitmap);
   }
   return result;
}

OperatorResult Intersection::evaluate() const {
   std::vector<OperatorResult> children_bm;
   children_bm.reserve(children.size());
   std::transform(
      children.begin(),
      children.end(),
      std::back_inserter(children_bm),
      [&](const auto& child) { return child->evaluate(); }
   );
   std::vector<OperatorResult> negated_children_bm;
   negated_children_bm.reserve(negated_children.size());
   std::transform(
      negated_children.begin(),
      negated_children.end(),
      std::back_inserter(negated_children_bm),
      [&](const auto& child) { return child->evaluate(); }
   );
   // Sort ascending, such that intermediate results are kept small
   std::sort(
      children_bm.begin(),
      children_bm.end(),
      [](const OperatorResult& expression1, const OperatorResult& expression2) {
         return expression1.getConst()->cardinality() < expression2.getConst()->cardinality();
      }
   );
   // Sort negated children descending by size
   std::sort(
      negated_children_bm.begin(),
      negated_children_bm.end(),
      [](const OperatorResult& expression_result1, const OperatorResult& expression_result2) {
         return expression_result1.getConst()->cardinality() >
                expression_result2.getConst()->cardinality();
      }
   );

   // children_bm > 0 as asserted in constructor
   if (children_bm.size() == 1) {
      // negated_children_bm cannot be empty because of size assertion in constructor
      roaring::Roaring* result = children_bm[0].getMutable();
      for (auto neg_bm : negated_children_bm) {
         *result -= *neg_bm.getConst();
         neg_bm.free();
      }
      return OperatorResult(result);
   }
   roaring::Roaring* result = intersectTwo(children_bm[0], children_bm[1]);
   for (unsigned i = 2; i < children.size(); i++) {
      auto bitmap = children_bm[i];
      *result &= *bitmap.getConst();
      bitmap.free();
   }
   for (auto neg_bm : negated_children_bm) {
      *result -= *neg_bm.getConst();
      neg_bm.free();
   }
   return OperatorResult(result);
}

}  // namespace silo::query_engine::operators
