#pragma once

#include <cassert>
#include <cstddef>
#include <string>

#include <fmt/format.h>

/// A range of values that support `-` yielding `size_t`, `+ (size_t)`
/// yielding `T`, `<=`, `++` and `==` (todo: find and use the concepts
/// for these).
template <typename T>
class Range {
   T first_;
   T beyond_last_;

  public:
   /// `first <= beyond_last` must be true.
   Range(T first, T beyond_last)
       : first_(first),
         beyond_last_(beyond_last) {
      assert(first <= beyond_last);
   }

   /// Get the first value of the Range. Unsafe, only valid if
   /// !isEmpty().
   T first() const { return first_; }

   /// Get the value one after the last one in the Range.
   T beyondLast() const { return beyond_last_; }

   /// Mutate this Range to drop the first element. Unsafe, only
   /// valid if !isEmpty().
   void mutRest() { first_++; }

   [[nodiscard]] std::string toString() const {
      return fmt::format("Range({}, {})", first_, beyond_last_);
   }

   [[nodiscard]] bool isEmpty() const { return first_ == beyond_last_; }

   [[nodiscard]] size_t size() const { return beyond_last_ - first_; }

   /// Skip the first `n` positions. If `n` is greater than `size()`,
   /// returns the empty range with `first()` and `beyondLast()` both
   /// being the original `beyondLast()`.
   [[nodiscard]] Range<T> drop(size_t n) const {
      if (n <= size()) {
         return {first_ + n, beyond_last_};
      }
      return {beyond_last_, beyond_last_};
   }

   /// Same as `drop` but mutating this Range in place.
   void mutDrop(size_t n) { *this = drop(n); }

   /// Take the first `n` positions. If `n` is greater than `size()`,
   /// returns the same range as this range.
   [[nodiscard]] Range<T> take(size_t n) const {
      auto siz = size();
      if (n <= siz) {
         return {first_, beyond_last_ - (siz - n)};
      }
      return {first_, beyond_last_};
   }

   struct Iterator {
      T value;

      Iterator& operator++() {
         ++value;
         return *this;
      }

      bool operator==(const Iterator& other) const { return value == other.value; }

      bool operator!=(const Iterator& other) const { return !(*this == other); }

      T operator*() const { return value; }
   };

   Iterator begin() const { return {first_}; }

   Iterator end() const { return {beyond_last_}; }
};
