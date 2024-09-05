#include "silo/common/optional_bool.h"

#include <cstdlib>

#include "silo/common/panic.h"

namespace silo::common {

using optional_bool::Representation;

OptionalBool::OptionalBool() {
   representation = Representation::NONE;
}

OptionalBool::OptionalBool(bool value) {
   representation = value ? Representation::TRUE : Representation::FALSE;
}

OptionalBool::OptionalBool(std::optional<bool> value) {
   if (value.has_value()) {
      representation = value ? Representation::TRUE : Representation::FALSE;
   } else {
      representation = Representation::NONE;
   }
}

std::strong_ordering OptionalBool::operator<=>(const OptionalBool& other) const {
   return representation <=> other.representation;
}

bool OptionalBool::operator==(const OptionalBool& other) const {
   return representation == other.representation;
}

bool OptionalBool::isNull() const noexcept {
   return representation == Representation::NONE;
}

std::optional<bool> OptionalBool::value() const noexcept {
   switch (representation) {
      case Representation::NONE:
         return std::nullopt;
      case Representation::FALSE:
         return false;
      case Representation::TRUE:
         return true;
   }
   UNREACHABLE();
}

std::string_view OptionalBool::asStr() const noexcept {
   switch (representation) {
      case Representation::NONE:
         return "null";
      case Representation::FALSE:
         return "false";
      case Representation::TRUE:
         return "true";
   }
   UNREACHABLE();
}

}  // namespace silo::common
