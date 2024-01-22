#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace silo {

std::vector<std::string> splitBy(const std::string& value, const std::string_view delimiter);

std::string removeSymbol(const std::string& value, char symbol);

std::vector<std::string> slice(const std::vector<std::string>& elements, size_t start, size_t end);

}  // namespace silo
