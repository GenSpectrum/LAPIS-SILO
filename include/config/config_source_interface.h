#pragma once

#include "config/config_specification.h"
#include "config/verified_config_attributes.h"

namespace silo::config {

template <typename T>
concept ConfigSource = requires(const T& obj, const ConfigSpecification& spec) {
   { obj.debugContext() } -> std::convertible_to<std::string>;

   /// The verify method checks that all found keys are OK and
   /// specified for the desired config type, parses their
   /// representation, and returns a VerifiedConfigAttributes object ready
   /// for retrieval of the values.
   { obj.verify(spec) } -> std::same_as<typename T::VerifiedType>;
};

}  // namespace silo::config
