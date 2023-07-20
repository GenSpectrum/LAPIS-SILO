#include "silo/query_engine/operators/selection.h"

#include <stdexcept>
#include <utility>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/date.h"
#include "silo/common/string.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

template <typename T>
Selection<T>::Selection(
   const std::vector<T>& column,
   Comparator comparator,
   T value,
   uint32_t row_count
)
    : column(column),
      comparator(comparator),
      value(std::move(value)),
      row_count(row_count) {
   if (row_count > this->column.size()) {
      throw std::runtime_error("Rows do not match vector size for Selection operator");
   }
}

template <typename T>
Selection<T>::~Selection() noexcept = default;

template <typename T>
std::string displayComparator(typename Selection<T>::Comparator comparator) {
   switch (comparator) {
      case Selection<T>::EQUALS:
         return "=";
      case Selection<T>::NOT_EQUALS:
         return "!=";
      case Selection<T>::LESS:
         return "<";
      case Selection<T>::HIGHER:
         return ">";
      case Selection<T>::LESS_OR_EQUALS:
         return "<=";
      case Selection<T>::HIGHER_OR_EQUALS:
         return ">=";
   }
   throw std::runtime_error("found unhandled comparator" + std::to_string(comparator));
}

template <typename T>
std::string Selection<T>::toString() const {
   return "Select" + displayComparator<T>(comparator);
}

template <typename T>
Type Selection<T>::type() const {
   return SELECTION;
}

template <typename T>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
OperatorResult Selection<T>::evaluate() const {
   auto* result = new roaring::Roaring();
   switch (this->comparator) {
      case EQUALS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] == value) {
               result->add(i);
            }
         }
         break;
      case NOT_EQUALS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] != value) {
               result->add(i);
            }
         }
         break;
      case LESS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] < value) {
               result->add(i);
            }
         }
         break;
      case HIGHER_OR_EQUALS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] >= value) {
               result->add(i);
            }
         }
         break;
      case HIGHER:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] > value) {
               result->add(i);
            }
         }
         break;
      case LESS_OR_EQUALS:
         for (uint32_t i = 0; i < row_count; i++) {
            if (column[i] <= value) {
               result->add(i);
            }
         }
         break;
   }
   return OperatorResult(result);
}

template <typename T>
std::unique_ptr<Operator> Selection<T>::copy() const {
   return std::make_unique<Selection<T>>(column, comparator, value, row_count);
}

template <typename T>
std::unique_ptr<Operator> Selection<T>::negate() const {
   Selection::Comparator new_comparator;
   switch (this->comparator) {
      case EQUALS:
         new_comparator = NOT_EQUALS;
         break;
      case NOT_EQUALS:
         new_comparator = EQUALS;
         break;
      case LESS:
         new_comparator = HIGHER_OR_EQUALS;
         break;
      case HIGHER_OR_EQUALS:
         new_comparator = LESS;
         break;
      case HIGHER:
         new_comparator = LESS_OR_EQUALS;
         break;
      case LESS_OR_EQUALS:
         new_comparator = HIGHER;
         break;
      default:
         throw std::runtime_error(
            "Unknown Selection comparator in negate method: " + std::to_string(comparator)
         );
   }
   return std::make_unique<Selection<T>>(column, new_comparator, value, row_count);
}

template class Selection<int32_t>;
template class Selection<silo::common::String<silo::common::STRING_SIZE>>;
template class Selection<silo::common::Date>;
template class Selection<double>;

}  // namespace silo::query_engine::operators
