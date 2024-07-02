#pragma once

#include <string>

namespace silo::common {

/// A range of values that support `-` yielding `size_t`, `+ (size_t)`
/// yielding `T`, `<=`, `++` and `==` (todo: find and use the concepts
/// for these).
///
/// Consider using `std::ranges::iota_view` instead if you don't need
/// the utility methods.
template <typename T>
class Range {
   T first_;
   T beyond_last_;

  public:
   /// `first <= beyond_last` must be true, otherwise throws a
   /// `std::runtime_error`.
   Range(T first, T beyond_last);

   /// Get the first value of the Range. Throws a `std::runtime_error`
   /// if isEmpty().
   T first() const;

   /// Get the value one after the last one in the Range.
   T beyondLast() const;

   /// Return the rest of the range after dropping the first
   /// element. Throws a `std::runtime_error` if isEmpty().
   [[nodiscard]] Range<T> skip1() const;

   [[nodiscard]] std::string toString() const;

   [[nodiscard]] bool isEmpty() const;

   [[nodiscard]] size_t size() const;

   /// Drop the first `n` positions. If `n` is greater than `size()`,
   /// returns the empty range with `first()` and `beyondLast()` both
   /// being the original `beyondLast()`.
   [[nodiscard]] Range<T> skip(size_t n) const;

   /// Take the first `n` positions. If `n` is greater than `size()`,
   /// returns the same range as this range.
   [[nodiscard]] Range<T> take(size_t n) const;

   /// For an Iterator type, see the commit in the Git history that
   /// changed *this* line.
};

}  // namespace silo::common
