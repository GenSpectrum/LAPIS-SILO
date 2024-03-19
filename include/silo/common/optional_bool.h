#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace silo::common {

namespace optional_bool {
enum class Representation : uint8_t { NONE = 0x00, FALSE = 0x80, TRUE = 0x81 };
}

class OptionalBool {
   optional_bool::Representation representation;

  public:
   explicit OptionalBool();
   explicit OptionalBool(bool value);
   explicit OptionalBool(std::optional<bool> value);

   std::strong_ordering operator<=>(const OptionalBool& other) const;

   [[nodiscard]] bool isNull() const noexcept;
   [[nodiscard]] std::optional<bool> value() const noexcept;

   [[nodiscard]] std::string_view asStr() const noexcept;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      archive & representation;
   }
};

}  // namespace silo::common
