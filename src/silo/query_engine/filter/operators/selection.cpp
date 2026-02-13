#include "silo/query_engine/filter/operators/selection.h"

#include <algorithm>
#include <cmath>
#include <compare>
#include <iterator>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/common/german_string.h"
#include "silo/common/panic.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

using storage::column::StringColumnPartition;

Selection::Selection(
   std::optional<std::unique_ptr<Operator>> child_operator,
   std::vector<std::unique_ptr<Predicate>>&& predicates,
   uint32_t row_count
)
    : child_operator(std::move(child_operator)),
      predicates(std::move(predicates)),
      row_count(row_count) {
   std::ranges::sort(this->predicates, [&row_count](const auto& left, const auto& right) {
      return left->estimateSelectivity(row_count) < right->estimateSelectivity(row_count);
   });
}

Selection::Selection(
   std::unique_ptr<Operator>&& child_operator,
   std::vector<std::unique_ptr<Predicate>>&& predicates,
   uint32_t row_count
)
    : Selection(
         std::optional<std::unique_ptr<Operator>>{std::move(child_operator)},
         std::move(predicates),
         row_count
      ) {}

Selection::Selection(
   std::unique_ptr<Operator>&& child_operator,
   std::unique_ptr<Predicate> predicate,
   uint32_t row_count
)
    : child_operator(std::move(child_operator)),
      row_count(row_count) {
   predicates.emplace_back(std::move(predicate));
}

Selection::Selection(std::vector<std::unique_ptr<Predicate>>&& predicates, uint32_t row_count)
    : Selection(std::nullopt, std::move(predicates), row_count) {}

Selection::Selection(std::unique_ptr<Predicate> predicate, uint32_t row_count)
    : row_count(row_count) {
   predicates.emplace_back(std::move(predicate));
}

Selection::~Selection() noexcept = default;

std::string Selection::toString() const {
   std::vector<std::string> predicate_strings;
   std::ranges::transform(
      predicates,
      std::back_inserter(predicate_strings),
      [](const auto& predicate) { return predicate->toString(); }
   );
   std::string child_operator_string;
   if (child_operator.has_value()) {
      child_operator_string = child_operator.value()->toString();
   }
   return fmt::format("Select[{}]({})", fmt::join(predicate_strings, ","), child_operator_string);
}

Type Selection::type() const {
   return SELECTION;
}

bool Selection::matchesPredicates(const PredicateVector& predicates, uint32_t row) {
   return std::ranges::all_of(predicates, [&](const auto& predicate) {
      return predicate->match(row);
   });
}

CopyOnWriteBitmap Selection::evaluate() const {
   EVOBENCH_SCOPE("Selection", "evaluate");
   if (child_operator.has_value()) {
      CopyOnWriteBitmap child_result = (*child_operator)->evaluate();
      // Do not iterate over bitmap, if child result is larger than 10%
      if (child_result.getConstReference().cardinality() > row_count / 10) {
         SILO_ASSERT(!predicates.empty());
         auto most_selective_predicate_bitmap = predicates.front()->makeBitmap(row_count);
         child_result.getMutable() &= most_selective_predicate_bitmap;

         if (predicates.size() == 1) {
            return CopyOnWriteBitmap{std::move(child_result)};
         }

         // Apply all remaining predicates as before `matchesPredicates`
         PredicateVector remaining_predicates;
         remaining_predicates.reserve(predicates.size() - 1);
         for (size_t i = 1; i < predicates.size(); ++i) {
            remaining_predicates.push_back(predicates.at(i)->copy());
         }
         roaring::Roaring result;
         for (const uint32_t row : child_result.getConstReference()) {
            if (matchesPredicates(remaining_predicates, row)) {
               result.add(row);
            }
         }
         return CopyOnWriteBitmap{std::move(result)};
      }
      roaring::Roaring result;
      for (const uint32_t row : child_result.getConstReference()) {
         if (matchesPredicates(predicates, row)) {
            result.add(row);
         }
      }
      return CopyOnWriteBitmap{std::move(result)};
   }
   roaring::Roaring result;
   for (uint32_t row = 0; row < row_count; ++row) {
      if (matchesPredicates(predicates, row)) {
         result.add(row);
      }
   }
   return CopyOnWriteBitmap{std::move(result)};
}

std::unique_ptr<Operator> Selection::negate(std::unique_ptr<Selection>&& selection) {
   const uint32_t row_count = selection->row_count;
   if (selection->child_operator == std::nullopt && selection->predicates.size() == 1) {
      return std::make_unique<Selection>(selection->predicates.at(0)->negate(), row_count);
   }
   return std::make_unique<Complement>(std::move(selection), row_count);
}

namespace {

bool strongOrderingMatchesComparator(std::strong_ordering strong_ordering, Comparator comparator) {
   if (strong_ordering == std::strong_ordering::equal) {
      return comparator == Comparator::HIGHER_OR_EQUALS ||
             comparator == Comparator::LESS_OR_EQUALS || comparator == Comparator::EQUALS;
   }
   if (strong_ordering == std::strong_ordering::less) {
      return comparator == Comparator::LESS || comparator == Comparator::LESS_OR_EQUALS ||
             comparator == Comparator::NOT_EQUALS;
   }
   if (strong_ordering == std::strong_ordering::greater) {
      return comparator == Comparator::HIGHER || comparator == Comparator::HIGHER_OR_EQUALS ||
             comparator == Comparator::NOT_EQUALS;
   }
   SILO_UNREACHABLE();
}

}  // namespace

template <>
bool CompareToValueSelection<StringColumnPartition>::match(uint32_t row_id) const {
   if (column.isNull(row_id)) {
      return with_nulls;
   }

   SiloString row_value = column.getValue(row_id);

   auto fast_compare = row_value.fastCompare(value);
   if (fast_compare.has_value()) {
      return strongOrderingMatchesComparator(fast_compare.value(), comparator);
   }

   auto row_value_string = column.lookupValue(row_value);

   // Slower fall-back if the prefix could not decide the match
   std::strong_ordering strong_ordering = std::lexicographical_compare_three_way(
      row_value_string.begin(), row_value_string.end(), value.begin(), value.end()
   );
   return strongOrderingMatchesComparator(strong_ordering, comparator);
}

}  // namespace silo::query_engine::filter::operators
