#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace silo::common {

using Date = int32_t;

std::expected<Date, std::string> stringToDate(std::string_view value);

std::string dateToString(Date date);

}  // namespace silo::common
