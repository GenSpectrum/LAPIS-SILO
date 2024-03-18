#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace silo::common {

using JsonValueType = std::optional<std::variant<std::string, int32_t, double>>;

}
