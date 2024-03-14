#pragma once

#include <cstdint>
#include <optional>

namespace silo::common {

class OptionalBool {
    uint8_t flag_and_value;
public:
    OptionalBool(std::optional<bool> value);
    bool isNull() const noexcept;
    std::optional<bool> value() const noexcept;
};


}
