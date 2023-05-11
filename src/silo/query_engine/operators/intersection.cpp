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
}

Intersection::~Intersection() noexcept = default;

std::string Intersection::toString(const Database& database) const {
   std::string res = "(" + this->children[0]->toString(database);
   for (const auto& child : children) {
      res += " & " + child->toString(database);
   }
   for (const auto& child : negated_children) {
      res += " &! " + child->toString(database);
   }
   res += ")";
   return res;
}

Type Intersection::type() const {
   return INTERSECTION;
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
   /// Sort ascending, such that intermediate results are kept small
   std::sort(
      children_bm.begin(),
      children_bm.end(),
      [](const OperatorResult& expression1, const OperatorResult& expression2) {
         return expression1.getAsConst()->cardinality() < expression2.getAsConst()->cardinality();
      }
   );

   roaring::Roaring* result;
   /// children_bm > 0 as asserted in cnstructor
   if (children_bm.size() == 1) {
      if (negated_children_bm.empty()) {
         throw std::runtime_error(
            "Error during 'And' evaluation: negated children were empty although their was only "
            "one non-negated child."
         );
      }
      if (children_bm[0].mutable_res) {
         result = children_bm[0].mutable_res;
      } else {
         auto tmp = *children_bm[0].immutable_res - *negated_children_bm[0].getAsConst();
         result = new roaring::Roaring(tmp);
      }
      /// Sort negated children descending by size
      std::sort(
         negated_children_bm.begin(),
         negated_children_bm.end(),
         [](const OperatorResult& expression_result1, const OperatorResult& expression_result2) {
            return expression_result1.getAsConst()->cardinality() >
                   expression_result2.getAsConst()->cardinality();
         }
      );
      for (auto neg_bm : negated_children_bm) {
         *result -= *neg_bm.getAsConst();
         neg_bm.free();
      }
      return {result, nullptr};
   }
   if (children_bm[0].mutable_res) {
      result = children_bm[0].mutable_res;
      *result &= *children_bm[1].getAsConst();
      children_bm[1].free();
   } else if (children_bm[1].mutable_res) {
      result = children_bm[1].mutable_res;
      *result &= *children_bm[0].getAsConst();
   } else {
      const auto bitmap = *children_bm[0].immutable_res & *children_bm[1].immutable_res;
      result = new roaring::Roaring(bitmap);
   }
   for (unsigned i = 2; i < children.size(); i++) {
      auto bitmap = children_bm[i];
      *result &= *bitmap.getAsConst();
      bitmap.free();
   }
   /// Sort negated children descending by size
   std::sort(
      negated_children_bm.begin(),
      negated_children_bm.end(),
      [](const OperatorResult& expression_result1, const OperatorResult& expression_result2) {
         return expression_result1.getAsConst()->cardinality() >
                expression_result2.getAsConst()->cardinality();
      }
   );
   for (auto neg_bm : negated_children_bm) {
      *result -= *neg_bm.getAsConst();
      neg_bm.free();
   }
   return OperatorResult{result, nullptr};
}

}  // namespace silo::query_engine::operators
