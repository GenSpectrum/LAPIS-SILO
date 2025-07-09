#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace silo::common {

typedef uint32_t Date;

const Date NULL_DATE = 0;

silo::common::Date stringToDate(std::string_view value);

std::optional<std::string> dateToString(silo::common::Date date);

}  // namespace silo::common
