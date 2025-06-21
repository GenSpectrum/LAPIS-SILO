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

#include "silo/common/date.h"
#include "silo/common/optional_bool.h"
#include "silo/common/panic.h"
#include "silo/common/string.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

using silo::common::OptionalBool;

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

namespace {
std::string displayComparator(Comparator comparator) {
   switch (comparator) {
      case Comparator::EQUALS:
         return "=";
      case Comparator::NOT_EQUALS:
         return "!=";
      case Comparator::LESS:
         return "<";
      case Comparator::HIGHER:
         return ">";
      case Comparator::LESS_OR_EQUALS:
         return "<=";
      case Comparator::HIGHER_OR_EQUALS:
         return ">=";
   }
   throw std::runtime_error("found unhandled comparator");
}
}  // namespace

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
   CopyOnWriteBitmap result;
   if (child_operator.has_value()) {
      CopyOnWriteBitmap child_result = (*child_operator)->evaluate();
      for (const uint32_t row : *child_result) {
         if (matchesPredicates(row)) {
            result->add(row);
         }
      }
   } else {
      for (uint32_t row = 0; row < row_count; ++row) {
         if (matchesPredicates(row)) {
            result->add(row);
         }
      }
   }
   return result;
}

std::unique_ptr<Operator> Selection::negate(std::unique_ptr<Selection>&& selection) {
   const uint32_t row_count = selection->row_count;
   if (selection->child_operator == std::nullopt && selection->predicates.size() == 1) {
      return std::make_unique<Selection>(selection->predicates.at(0)->negate(), row_count);
   }
   return std::make_unique<Complement>(std::move(selection), row_count);
}

template <typename T>
CompareToValueSelection<T>::CompareToValueSelection(
   const std::vector<T>& column,
   Comparator comparator,
   T value
)
    : column(column),
      comparator(comparator),
      value(value) {}

template <typename T>
bool CompareToValueSelection<T>::match(uint32_t row_id) const {
   SILO_ASSERT(column.size() > row_id);
   switch (comparator) {
      case Comparator::EQUALS:
         return column[row_id] == value;
      case Comparator::NOT_EQUALS:
         return column[row_id] != value;
      case Comparator::LESS:
         return column[row_id] < value;
      case Comparator::HIGHER_OR_EQUALS:
         return column[row_id] >= value;
      case Comparator::HIGHER:
         return column[row_id] > value;
      case Comparator::LESS_OR_EQUALS:
         return column[row_id] <= value;
   }
   throw std::runtime_error(
      "Uncovered enum switch case in CompareToValueSelection<T>::match should be covered by linter."
   );
}

template <>
bool CompareToValueSelection<double>::match(uint32_t row_id) const {
   SILO_ASSERT(column.size() > row_id);
   switch (comparator) {
      case Comparator::EQUALS:
         if (std::isnan(value)) {
            return std::isnan(column[row_id]);
         }
         return column[row_id] == value;
      case Comparator::NOT_EQUALS:
         if (std::isnan(value)) {
            return !std::isnan(column[row_id]);
         }
         return column[row_id] != value;
      case Comparator::LESS:
         return column[row_id] < value;
      case Comparator::HIGHER_OR_EQUALS:
         return column[row_id] >= value;
      case Comparator::HIGHER:
         return column[row_id] > value;
      case Comparator::LESS_OR_EQUALS:
         return column[row_id] <= value;
   }
   throw std::runtime_error(
      "Uncovered enum switch case in CompareToValueSelection<double>::match should be covered by "
      "linter."
   );
}

template <>
bool CompareToValueSelection<silo::common::SiloString>::match(uint32_t row_id) const {
   SILO_ASSERT(column.size() > row_id);
   if (comparator == Comparator::EQUALS) {
      return column[row_id] == value;
   }
   if (comparator == Comparator::NOT_EQUALS) {
      return column[row_id] != value;
   }

   const silo::common::SiloString& row_value = column.at(row_id);

   auto fast_compare = row_value.fastCompare(value);
   if (fast_compare) {
      if (*fast_compare == std::strong_ordering::equal) {
         return comparator == Comparator::HIGHER_OR_EQUALS ||
                comparator == Comparator::LESS_OR_EQUALS;
      }
      if (*fast_compare == std::strong_ordering::less) {
         return comparator == Comparator::LESS || comparator == Comparator::LESS_OR_EQUALS;
      }
      if (*fast_compare == std::strong_ordering::greater) {
         return comparator == Comparator::HIGHER || comparator == Comparator::HIGHER_OR_EQUALS;
      }
   }
   // TODO(#137)
   return true;
}

template <typename T>
std::unique_ptr<Predicate> CompareToValueSelection<T>::copy() const {
   return std::make_unique<CompareToValueSelection<T>>(column, comparator, value);
}

template <typename T>
[[nodiscard]] std::unique_ptr<Predicate> CompareToValueSelection<T>::negate() const {
   Comparator negated_comparator;
   switch (comparator) {
      case Comparator::EQUALS:
         negated_comparator = Comparator::NOT_EQUALS;
         break;
      case Comparator::NOT_EQUALS:
         negated_comparator = Comparator::EQUALS;
         break;
      case Comparator::LESS:
         negated_comparator = Comparator::HIGHER_OR_EQUALS;
         break;
      case Comparator::HIGHER_OR_EQUALS:
         negated_comparator = Comparator::LESS;
         break;
      case Comparator::HIGHER:
         negated_comparator = Comparator::LESS_OR_EQUALS;
         break;
      case Comparator::LESS_OR_EQUALS:
         negated_comparator = Comparator::HIGHER;
         break;
      default:
         throw std::logic_error("Unknown comparator in negate()");
   }
   return std::make_unique<CompareToValueSelection<T>>(column, negated_comparator, value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<OptionalBool>::toString() const {
   return fmt::format("$bool {} {}", displayComparator(comparator), value.asStr());
}

template <>
[[nodiscard]] std::string CompareToValueSelection<int32_t>::toString() const {
   return "$int " + displayComparator(comparator) + " " + std::to_string(value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<common::SiloString>::toString() const {
   std::stringstream stream;
   stream << "$string " << displayComparator(comparator) << " " << value.dataAsHexString();
   return stream.str();
}

template <>
[[nodiscard]] std::string CompareToValueSelection<std::string>::toString() const {
   return "$string " + displayComparator(comparator) + " " + value;
}

template <>
[[nodiscard]] std::string CompareToValueSelection<silo::common::Date>::toString() const {
   return "$date " + displayComparator(comparator) + " " + std::to_string(value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<double>::toString() const {
   return "$double " + displayComparator(comparator) + " " + std::to_string(value);
}

template class CompareToValueSelection<OptionalBool>;
template class CompareToValueSelection<int32_t>;
template class CompareToValueSelection<common::SiloString>;
template class CompareToValueSelection<silo::common::Date>;
template class CompareToValueSelection<double>;

}  // namespace silo::query_engine::filter::operators
