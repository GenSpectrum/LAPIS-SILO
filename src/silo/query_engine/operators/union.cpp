#include "silo/query_engine/operators/union.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/operator.h"

using silo::query_engine::filter_expressions::Expression;
using silo::query_engine::filter_expressions::PositionalFilter;

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

bool Union::isDisjointUnion() const {
   assert(!children.empty());
   if (std::any_of(children.begin(), children.end(), [](auto& child) {
          return child->logicalEquivalent() == std::nullopt;
       })) {
      return false;
   }
   const std::optional<PositionalFilter> matching_positional_for_all_children =
      Expression::isPositionalFilterForSymbol(*children.at(0)->logicalEquivalent());
   if (matching_positional_for_all_children == std::nullopt) {
      return false;
   }
   const auto sequence_name = matching_positional_for_all_children->sequence_name;
   const uint32_t position = matching_positional_for_all_children->position;
   const bool is_nucleotide =
      holds_alternative<Nucleotide::Symbol>(matching_positional_for_all_children->symbol);
   std::set<Nucleotide::Symbol> nuc_symbols;
   std::set<AminoAcid::Symbol> aa_symbols;
   const bool all_positional_filters_for_same_position =
      std::all_of(children.begin(), children.end(), [&](auto& child) {
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
      });
   return all_positional_filters_for_same_position &&
          (nuc_symbols.size() == children.size() || aa_symbols.size() == children.size());
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

std::unique_ptr<Operator> Union::negate(std::unique_ptr<Union>&& union_operator) {
   return std::make_unique<Complement>(std::move(union_operator), union_operator->row_count);
}

}  // namespace silo::query_engine::operators
