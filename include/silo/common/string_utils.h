#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace silo {

std::vector<std::string> splitBy(const std::string& value, const std::string_view delimiter);

}  // namespace silo
