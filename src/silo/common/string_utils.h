#pragma once

#include <fmt/core.h>
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace silo {

std::vector<std::string> splitBy(const std::string& value, std::string_view delimiter);

std::string removeSymbol(const std::string& value, char symbol);

std::vector<std::string> slice(const std::vector<std::string>& elements, size_t start, size_t end);

std::vector<std::string> prepend(std::string_view prefix, const std::vector<std::string>& elements);

std::vector<std::string> tie(
   std::string_view prefix,
   const std::vector<std::string>& elements1,
   std::string_view delimiter,
   const std::vector<std::string>& elements2,
   std::string_view suffix
);

std::string tieAsString(
   std::string_view prefix,
   const std::vector<std::string>& elements1,
   std::string_view delimiter,
   const std::vector<std::string>& elements2,
   std::string_view suffix
);

template <typename T>
std::string joinWithLimit(
   const std::vector<T>& items,
   std::string_view delimiter = ", ",
   size_t limit = 10
) {
   std::string res;
   const size_t items_to_print = std::min(items.size(), limit);

   for (size_t i = 0; i < items_to_print; ++i) {
      if (i > 0) {
         res += delimiter;
      }
      // Assumes items[i] has a toString() method or works with fmt
      res += items[i]->toString();
   }

   if (items.size() > items_to_print) {
      res += fmt::format("{}... ({} more)", delimiter, items.size() - items_to_print);
   }

   return res;
}

}  // namespace silo
