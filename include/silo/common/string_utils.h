#ifndef SILO_STRING_UTILS_H
#define SILO_STRING_UTILS_H

#include <string>
#include <string_view>
#include <vector>

namespace silo {

std::vector<std::string> splitBy(const std::string& value, const std::string_view delimiter);

}  // namespace silo

#endif  // SILO_STRING_UTILS_H
