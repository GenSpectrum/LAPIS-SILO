#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace rhydb::common {

using JsonValueType = std::optional<std::variant<std::string, bool, int32_t, double>>;

}
