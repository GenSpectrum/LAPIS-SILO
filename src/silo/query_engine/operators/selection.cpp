#include "silo/query_engine/operators/selection.h"

#include <roaring/roaring.hh>
#include <utility>
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

template <typename T>
Selection<T>::Selection(const std::vector<T>& column, Comparator comparator, T value)
    : column(column),
      comparator(comparator),
      value(std::move(value)) {}

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
   return "Select-" + displayComparator<T>(comparator);
}

template <typename T>
Type Selection<T>::type() const {
   return SELECTION;
}

template <typename T>
void Selection<T>::negate() {
   switch (this->comparator) {
      case EQUALS:
         this->comparator = NOT_EQUALS;
         break;
      case NOT_EQUALS:
         this->comparator = EQUALS;
         break;
      case LESS:
         this->comparator = HIGHER_OR_EQUALS;
         break;
      case HIGHER_OR_EQUALS:
         this->comparator = LESS;
         break;
      case HIGHER:
         this->comparator = LESS_OR_EQUALS;
         break;
      case LESS_OR_EQUALS:
         this->comparator = HIGHER;
         break;
   }
}

template <typename T>
OperatorResult Selection<T>::evaluate() const {
   const auto size = column.size();

   auto* result = new roaring::Roaring();
   switch (this->comparator) {
      case EQUALS:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] == value) {
               result->add(i);
            }
         }
         break;
      case NOT_EQUALS:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] != value) {
               result->add(i);
            }
         }
         break;
      case LESS:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] < value) {
               result->add(i);
            }
         }
         break;
      case HIGHER_OR_EQUALS:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] >= value) {
               result->add(i);
            }
         }
         break;
      case HIGHER:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] > value) {
               result->add(i);
            }
         }
         break;
      case LESS_OR_EQUALS:
         for (unsigned i = 0; i < size; i++) {
            if (column[i] <= value) {
               result->add(i);
            }
         }
         break;
   }
   return OperatorResult(result);
}

template class Selection<std::string>;
template class Selection<uint64_t>;
template class Selection<time_t>;

}  // namespace silo::query_engine::operators
