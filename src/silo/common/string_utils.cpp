#include "silo/common/string_utils.h"

#include <algorithm>
#include <stdexcept>

namespace silo {

std::vector<std::string> splitBy(const std::string& value, const std::string_view delimiter) {
   std::vector<std::string> splits;

   std::string_view value_sv(value.c_str(), value.size());
   auto next_split_point = value_sv.find(delimiter);

   while (next_split_point != std::string::npos) {
      splits.emplace_back(value_sv.substr(0, next_split_point));
      value_sv = value_sv.substr(next_split_point + delimiter.size());
      next_split_point = value_sv.find(delimiter);
   }

   splits.emplace_back(value_sv);

   return splits;
}

std::string removeSymbol(const std::string& value, char symbol) {
   std::string result = value;
   result.erase(std::remove(result.begin(), result.end(), symbol), result.end());
   return result;
}

std::vector<std::string> slice(const std::vector<std::string>& elements, size_t start, size_t end) {
   std::vector<std::string> sliced_elements;

   if (end > elements.size()) {
      throw std::out_of_range("End index is larger than the size of the vector");
   }

   for (auto i = start; i < end; i++) {
      sliced_elements.emplace_back(elements.at(i));
   }
   return sliced_elements;
}
}  // namespace silo