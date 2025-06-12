#pragma once

//! Cons list. The tail of the list is stored as a normal reference
//! (no reference counting). This is meant to be used with recursive
//! algorithms to maintain a path back up.

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>

template <typename T>
class ConsList {
   std::optional<std::pair<T, std::reference_wrapper<const ConsList<T>>>> inner;

  public:
   explicit ConsList()
       : inner(std::nullopt) {}
   explicit ConsList(std::optional<std::pair<T, std::reference_wrapper<const ConsList<T>>>> inner_)
       : inner(inner_) {}

   ConsList<T> cons(T val) const {
      std::pair<T, const ConsList<T>&> pair{val, *this};
      return ConsList<T>(std::optional{pair});
   }

   [[nodiscard]] bool isEmpty() const { return !inner.has_value(); }

   std::optional<std::reference_wrapper<const T>> first() const {
      if (isEmpty()) {
         return std::nullopt;
      }
      const std::pair<T, std::reference_wrapper<const ConsList<T>>>& pair = inner.value();
      return std::optional<std::reference_wrapper<const T>>(std::get<0>(pair));
   }

   std::optional<std::reference_wrapper<const ConsList<T>>> rest() const {
      if (isEmpty()) {
         return std::nullopt;
      }
      const std::pair<T, std::reference_wrapper<const ConsList<T>>>& pair = inner.value();
      return std::optional<std::reference_wrapper<const ConsList<T>>>(std::get<1>(pair));
   }

   /* ... */

   // template<typename T /* : Clone */>
   std::vector<T> toVec() const {
      std::vector<T> values{};
      std::reference_wrapper<const ConsList<T>> current = std::cref(*this);

      while (!current.get().isEmpty()) {
         values.push_back(current.get().first().value());
         current = current.get().rest().value();
      }
      return values;
   }

   // template<typename T /* : Clone */>
   std::vector<T> toVecReverse() const {
      // There's no faster way than reverse (except perhaps
      // recursion, but that is dicey, or getting the list first
      // then set Vec slots via index, but that needs Default and
      // writes to memory twice, too), right?
      std::vector<T> values = toVec();
      std::ranges::reverse(values);
      return values;
   }
};
