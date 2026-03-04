#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace silo::common {

using Date32 = int32_t;

std::expected<Date32, std::string> stringToDate32(std::string_view value);

std::string date32ToString(Date32 date);

}  // namespace silo::common
