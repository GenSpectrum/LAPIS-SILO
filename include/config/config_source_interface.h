#pragma once

#include "config/config_specification.h"
#include "config/verified_config_source.h"

namespace silo::config {

class ConfigSource {
   [[nodiscard]] virtual std::string debugContext() const = 0;

  public:
   /// The verify method checks that all found keys are OK and
   /// specified for the desired config type, parses their
   /// representation, and returns a VerifiedConfigSource object ready
   /// for retrieval of the values.
   [[nodiscard]] virtual VerifiedConfigSource verify(const ConfigSpecification& config_specification
   ) const = 0;
};

}  // namespace silo::config
