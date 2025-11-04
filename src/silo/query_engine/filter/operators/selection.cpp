#include "silo/query_engine/filter/operators/selection.h"

#include <array>
#include <cmath>
#include <compare>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/common/date.h"
#include "silo/common/german_string.h"
#include "silo/common/panic.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

using storage::column::StringColumnPartition;

Selection::Selection(
   std::unique_ptr<Operator>&& child_operator,
   std::vector<std::unique_ptr<Predicate>>&& predicates,
   uint32_t row_count
)
    : child_operator(std::move(child_operator)),
      predicates(std::move(predicates)),
      row_count(row_count) {}

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
    : predicates(std::move(predicates)),
      row_count(row_count) {}

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
   return "Select[" + boost::algorithm::join(predicate_strings, ",") + "](" + ")";
}

Type Selection::type() const {
   return SELECTION;
}

bool Selection::matchesPredicates(uint32_t row) const {
   return std::ranges::all_of(predicates, [&](const auto& predicate) {
      return predicate->match(row);
   });
}

CopyOnWriteBitmap Selection::evaluate() const {
   EVOBENCH_SCOPE("Selection", "evaluate");
   roaring::Roaring result;
   if (child_operator.has_value()) {
      CopyOnWriteBitmap child_result = (*child_operator)->evaluate();
      for (const uint32_t row : child_result.getConstReference()) {
         if (matchesPredicates(row)) {
            result.add(row);
         }
      }
   } else {
      for (uint32_t row = 0; row < row_count; ++row) {
         if (matchesPredicates(row)) {
            result.add(row);
         }
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
   } else if (strong_ordering == std::strong_ordering::less) {
      return comparator == Comparator::LESS || comparator == Comparator::LESS_OR_EQUALS ||
             comparator == Comparator::NOT_EQUALS;
   } else if (strong_ordering == std::strong_ordering::greater) {
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
