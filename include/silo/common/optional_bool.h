#pragma once

#include <cstdint>
#include <optional>

namespace silo::common {

class OptionalBool {
    uint8_t flag_and_value; // bit 7: flag, bit 0: bool if flag is
                            // true, use 0x00 for null 0x80 for false,
                            // 0x81 for true
public:
    
    explicit OptionalBool();
    explicit OptionalBool(bool value);
    explicit OptionalBool(std::optional<bool> value);
    bool isNull() const noexcept;
    std::optional<bool> value() const noexcept;
};


}
