#include "silo/query_engine/filter/operators/intersection.h"

#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/query_compilation_exception.h"

namespace silo::query_engine::filter::operators {

Intersection::Intersection(
   OperatorVector&& children,
   OperatorVector&& negated_children,
   uint32_t row_count
)
    : children(std::move(children)),
      negated_children(std::move(negated_children)),
      row_count(row_count) {
   if (this->children.empty()) {
      SPDLOG_ERROR(
         "Compilation bug: Intersection without non-negated children is not allowed. "
         "Should be compiled as a union."
      );
      throw QueryCompilationException(
         "Compilation bug: Intersection without non-negated children is not allowed. "
         "Should be compiled as a union."
      );
   }
   if (this->children.size() + this->negated_children.size() < 2) {
      SPDLOG_ERROR("Compilation bug: Intersection needs at least two children.");

      throw QueryCompilationException("Compilation bug: Intersection needs at least two children.");
   }
}

Intersection::~Intersection() noexcept = default;

std::string Intersection::toString() const {
   std::string res = "(" + children[0]->toString();

   for (uint32_t i = 1; i < children.size(); i++) {
      res += " & " + children[i]->toString();
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

namespace {

CopyOnWriteBitmap intersectTwo(CopyOnWriteBitmap first, CopyOnWriteBitmap second) {
   CopyOnWriteBitmap result;
   if (first.isMutable()) {
      result = std::move(first);
      result.getMutable() &= second.getConstReference();
   } else if (second.isMutable()) {
      result = std::move(second);
      result.getMutable() &= first.getConstReference();
   } else {
      result = CopyOnWriteBitmap(first.getConstReference() & second.getConstReference());
   }
   return result;
}

}  // namespace

CopyOnWriteBitmap Intersection::evaluate() const {
   EVOBENCH_SCOPE("Intersection", "evaluate");
   std::vector<CopyOnWriteBitmap> children_bm;
   children_bm.reserve(children.size());
   std::ranges::transform(children, std::back_inserter(children_bm), [&](const auto& child) {
      return child->evaluate();
   });
   std::vector<CopyOnWriteBitmap> negated_children_bm;
   negated_children_bm.reserve(negated_children.size());
   std::ranges::transform(
      negated_children,
      std::back_inserter(negated_children_bm),
      [&](const auto& child) { return child->evaluate(); }
   );
   // Sort ascending, such that intermediate results are kept small
   std::ranges::sort(
      children_bm,
      [](const CopyOnWriteBitmap& expression1, const CopyOnWriteBitmap& expression2) {
         return expression1.getConstReference().cardinality() <
                expression2.getConstReference().cardinality();
      }
   );
   // Sort negated children descending by size
   std::ranges::sort(
      negated_children_bm,
      [](const CopyOnWriteBitmap& expression_result1, const CopyOnWriteBitmap& expression_result2) {
         return expression_result1.getConstReference().cardinality() >
                expression_result2.getConstReference().cardinality();
      }
   );

   // children_bm > 0 as asserted in constructor
   if (children_bm.size() == 1) {
      // negated_children_bm cannot be empty because of size assertion in constructor
      CopyOnWriteBitmap& result = children_bm[0];
      for (auto& neg_bm : negated_children_bm) {
         result.getMutable() -= neg_bm.getConstReference();
      }
      return std::move(result);
   }
   auto result = intersectTwo(std::move(children_bm[0]), std::move(children_bm[1]));
   for (uint32_t i = 2; i < children.size(); i++) {
      result.getMutable() &= children_bm[i].getConstReference();
   }
   for (auto& neg_bm : negated_children_bm) {
      result.getMutable() -= neg_bm.getConstReference();
   }
   return result;
}

std::unique_ptr<Operator> Intersection::negate(std::unique_ptr<Intersection>&& intersection) {
   const uint32_t row_count = intersection->row_count;
   return std::make_unique<Complement>(std::move(intersection), row_count);
}

}  // namespace silo::query_engine::filter::operators
