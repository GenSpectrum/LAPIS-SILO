#include "silo/query_engine/operators/selection.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <roaring/roaring.hh>

#include "silo/common/date.h"
#include "silo/common/string.h"
#include "silo/query_engine/operators/complement.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

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

std::string Selection::toString() const {
   std::vector<std::string> predicate_strings;
   std::transform(
      predicates.begin(),
      predicates.end(),
      std::back_inserter(predicate_strings),
      [](const auto& predicate) { return predicate->toString(); }
   );
   return "Select[" + boost::algorithm::join(predicate_strings, ",") + "](" + ")";
}

Type Selection::type() const {
   return SELECTION;
}

bool Selection::matchesPredicates(uint32_t row) const {
   return std::all_of(predicates.begin(), predicates.end(), [&](const auto& predicate) {
      return predicate->match(row);
   });
}

OperatorResult Selection::evaluate() const {
   auto* result = new roaring::Roaring();
   if (child_operator.has_value()) {
      for (const uint32_t row : *(*child_operator)->evaluate()) {
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
   return OperatorResult(result);
}

std::unique_ptr<Operator> Selection::copy() const {
   std::vector<std::unique_ptr<Predicate>> copied_predicates;
   std::transform(
      predicates.begin(),
      predicates.end(),
      std::back_inserter(copied_predicates),
      [](const auto& predicate) { return predicate->copy(); }
   );
   if (child_operator.has_value()) {
      return std::make_unique<Selection>(
         (*child_operator)->copy(), std::move(copied_predicates), row_count
      );
   }
   return std::make_unique<Selection>(std::move(copied_predicates), row_count);
}

std::unique_ptr<Operator> Selection::negate() const {
   if (child_operator == std::nullopt && predicates.size() == 1) {
      return std::make_unique<Selection>(predicates.at(0)->negate(), row_count);
   }
   return std::make_unique<Complement>(this->copy(), row_count);
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
   }
   return std::make_unique<CompareToValueSelection<T>>(column, negated_comparator, value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<int32_t>::toString() const {
   return "$string " + displayComparator(comparator) + " " + std::to_string(value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<common::SiloString>::toString() const {
   std::stringstream stream;
   stream << "$string " << displayComparator(comparator) << "0x" << std::setfill('0')
          << std::setw(static_cast<int>(value.data.size() * 2)) << std::hex << value.data.data();
   return stream.str();
}

template <>
[[nodiscard]] std::string CompareToValueSelection<std::string>::toString() const {
   return "$string " + displayComparator(comparator) + " " + value;
}

template <>
[[nodiscard]] std::string CompareToValueSelection<silo::common::Date>::toString() const {
   return "$string " + displayComparator(comparator) + " " + std::to_string(value);
}

template <>
[[nodiscard]] std::string CompareToValueSelection<double>::toString() const {
   return "$string " + displayComparator(comparator) + " " + std::to_string(value);
}

template class CompareToValueSelection<int32_t>;
template class CompareToValueSelection<common::SiloString>;
template class CompareToValueSelection<silo::common::Date>;
template class CompareToValueSelection<double>;

}  // namespace silo::query_engine::operators
