#include "silo/query_engine/operators/intersection.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/query_compilation_exception.h"

using silo::query_engine::filter_expressions::Expression;
using silo::query_engine::filter_expressions::PositionalFilter;

namespace silo::query_engine::operators {

Intersection::Intersection(
   std::vector<std::unique_ptr<Operator>>&& children,
   std::vector<std::unique_ptr<Operator>>&& negated_children,
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
      throw silo::QueryCompilationException(
         "Compilation bug: Intersection without non-negated children is not allowed. "
         "Should be compiled as a union."
      );
   }
   if (this->children.size() + this->negated_children.size() < 2) {
      SPDLOG_ERROR("Compilation bug: Intersection needs at least two children.");

      throw silo::QueryCompilationException(
         "Compilation bug: Intersection needs at least two children."
      );
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

bool Intersection::isNegatedDisjointUnion() const {
   assert(!children.empty() || !negated_children.empty());
   if (std::any_of(children.begin(), children.end(), [](auto& child) {
          return child->logicalEquivalent() == std::nullopt;
       })) {
      return false;
   }
   if (std::any_of(negated_children.begin(), negated_children.end(), [](auto& child) {
          return child->logicalEquivalent() == std::nullopt;
       })) {
      return false;
   }
   std::optional<PositionalFilter> matching_positional_for_all_children =
      children.empty()
         ? Expression::isPositionalFilterForSymbol(*negated_children.at(0)->logicalEquivalent())
         : Expression::isNegatedPositionalFilterForSymbol(*children.at(0)->logicalEquivalent());
   if (matching_positional_for_all_children == std::nullopt) {
      return false;
   }
   auto sequence_name = matching_positional_for_all_children->sequence_name;
   uint32_t position = matching_positional_for_all_children->position;
   bool is_nucleotide =
      holds_alternative<Nucleotide::Symbol>(matching_positional_for_all_children->symbol);
   std::set<Nucleotide::Symbol> nuc_symbols;
   std::set<AminoAcid::Symbol> aa_symbols;
   const bool all_positional_filters_for_same_position =
      std::all_of(
         negated_children.begin(),
         negated_children.end(),
         [&](auto& child) {
            auto test = Expression::isPositionalFilterForSymbol(*child->logicalEquivalent());
            if (test == std::nullopt) {
               return false;
            }
            if (holds_alternative<Nucleotide::Symbol>(test->symbol)) {
               nuc_symbols.insert(std::get<Nucleotide::Symbol>(test->symbol));
            } else {
               aa_symbols.insert(std::get<AminoAcid::Symbol>(test->symbol));
            }
            return test->sequence_name == sequence_name && test->position == position &&
                   holds_alternative<Nucleotide::Symbol>(test->symbol) == is_nucleotide;
         }
      ) &&
      std::all_of(children.begin(), children.end(), [&](auto& child) {
         auto test = Expression::isNegatedPositionalFilterForSymbol(*child->logicalEquivalent());
         if (test == std::nullopt) {
            return false;
         }
         if (holds_alternative<Nucleotide::Symbol>(test->symbol)) {
            nuc_symbols.insert(std::get<Nucleotide::Symbol>(test->symbol));
         } else {
            aa_symbols.insert(std::get<AminoAcid::Symbol>(test->symbol));
         }
         return test->sequence_name == sequence_name && test->position == position &&
                holds_alternative<Nucleotide::Symbol>(test->symbol) == is_nucleotide;
      });
   return all_positional_filters_for_same_position &&
          (nuc_symbols.size() == children.size() || aa_symbols.size() == children.size());
}

namespace {

OperatorResult intersectTwo(OperatorResult first, OperatorResult second) {
   OperatorResult result;
   if (first.isMutable()) {
      result = std::move(first);
      *result &= *second;
   } else if (second.isMutable()) {
      result = std::move(second);
      *result &= *first;
   } else {
      result = OperatorResult(*first & *second);
   }
   return result;
}

}  // namespace

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
         return expression1->cardinality() < expression2->cardinality();
      }
   );
   // Sort negated children descending by size
   std::sort(
      negated_children_bm.begin(),
      negated_children_bm.end(),
      [](const OperatorResult& expression_result1, const OperatorResult& expression_result2) {
         return expression_result1->cardinality() > expression_result2->cardinality();
      }
   );

   // children_bm > 0 as asserted in constructor
   if (children_bm.size() == 1) {
      // negated_children_bm cannot be empty because of size assertion in constructor
      OperatorResult& result = children_bm[0];
      for (auto& neg_bm : negated_children_bm) {
         *result -= *neg_bm;
      }
      return std::move(result);
   }
   auto result = intersectTwo(std::move(children_bm[0]), std::move(children_bm[1]));
   for (uint32_t i = 2; i < children.size(); i++) {
      *result &= *children_bm[i];
   }
   for (auto& neg_bm : negated_children_bm) {
      *result -= *neg_bm;
   }
   return result;
}

std::unique_ptr<Operator> Intersection::negate(std::unique_ptr<Intersection>&& intersection) {
   const uint32_t row_count = intersection->row_count;
   return std::make_unique<Complement>(std::move(intersection), row_count);
}

}  // namespace silo::query_engine::operators
