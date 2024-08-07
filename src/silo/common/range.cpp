#include "silo/common/range.h"

#include <cstddef>

#include <fmt/format.h>
#include <boost/numeric/conversion/cast.hpp>

namespace silo::common {

template <typename T>
Range<T>::Range(T first, T beyond_last)
    : first_(first),
      beyond_last_(beyond_last) {
   if (!(first <= beyond_last)) {
      throw std::runtime_error("can't make a range with first > beyond_last");
   }
}

template <typename T>
T Range<T>::first() const {
   if (isEmpty()) {
      throw std::runtime_error("can't get the first element of an empty range");
   }
   return first_;
}

template <typename T>
T Range<T>::beyondLast() const {
   return beyond_last_;
}

template <typename T>
[[nodiscard]] Range<T> Range<T>::skip1() const {
   if (isEmpty()) {
      throw std::runtime_error("can't get the rest of an empty range");
   }
   auto first = first_;
   first++;
   return Range{first, beyond_last_};
}

template <typename T>
[[nodiscard]] std::string Range<T>::toString() const {
   return fmt::format("Range({}, {})", first_, beyond_last_);
}

template <typename T>
[[nodiscard]] bool Range<T>::isEmpty() const {
   return first_ == beyond_last_;
}

template <typename T>
[[nodiscard]] size_t Range<T>::size() const {
   return beyond_last_ - first_;
}

template <typename T>
[[nodiscard]] Range<T> Range<T>::skip(size_t n) const {
   if (n <= size()) {
      return {first_ + boost::numeric_cast<T, size_t>(n), beyond_last_};
   }
   return {beyond_last_, beyond_last_};
}

template <typename T>
[[nodiscard]] Range<T> Range<T>::take(size_t n) const {
   const size_t siz = size();
   if (n <= siz) {
      return {first_, beyond_last_ - boost::numeric_cast<T, size_t>(siz - n)};
   }
   return {first_, beyond_last_};
}

template class Range<size_t>;
template class Range<unsigned int>;

}  // namespace silo::common
