#include "silo/common/optional_bool.h"

namespace silo::common {
    OptionalBool::OptionalBool(bool value) {
        flag_and_value = value ? 0x81 : 0x80;
    }
    
    OptionalBool::OptionalBool(std::optional<bool> value) {
        if (value.has_value()) {
            flag_and_value = value ? 0x81 : 0x80;
        } else {
            flag_and_value = 0x00;
        }
    }
    
    bool OptionalBool::isNull() const noexcept {
        return flag_and_value == 0;
    }
    
    std::optional<bool> OptionalBool::value() const noexcept {
        return flag_and_value & 0x80 ? std::optional<bool>(flag_and_value & 1) : std::nullopt;
    }

}  // namespace silo::common
