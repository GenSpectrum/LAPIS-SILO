#include "silo/common/string_utils.h"

#include <stdexcept>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>
#include "silo/common/panic.h"

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
   std::erase(result, symbol);
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

std::vector<std::string> prepend(
   std::string_view prefix,
   const std::vector<std::string>& elements
) {
   std::vector<std::string> output;
   output.reserve(elements.size());
   for (const std::string& str : elements) {
      output.emplace_back(fmt::format("{}{}", prefix, str));
   }
   return output;
}

std::vector<std::string> tie(
   std::string_view prefix,
   const std::vector<std::string>& elements1,
   std::string_view delimiter,
   const std::vector<std::string>& elements2,
   std::string_view suffix
) {
   SILO_ASSERT(elements1.size() == elements2.size());
   std::vector<std::string> output;
   output.reserve(elements1.size());
   for (size_t i = 0; i < elements1.size(); ++i) {
      output.emplace_back(
         fmt::format("{}{}{}{}{}", prefix, elements1.at(i), delimiter, elements2.at(i), suffix)
      );
   }
   return output;
}

std::string tieAsString(
   std::string_view prefix,
   const std::vector<std::string>& elements1,
   std::string_view delimiter,
   const std::vector<std::string>& elements2,
   std::string_view suffix
) {
   return boost::join(tie(prefix, elements1, delimiter, elements2, suffix), "");
}

}  // namespace silo
