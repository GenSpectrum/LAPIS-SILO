#pragma once

#include <string>

#include <config/config_source_interface.h>

class OverwriteFrom {
  public:
   /// Overwrite the fields of an instance of the target type; done
   /// that way so that multiple kinds of config sources can shadow
   /// each other's values by application in sequence. `parents` is
   /// the upwards path to the root of the struct tree (use
   /// .to_vec_reverse() and wrap in ConfigKeyPath). Throws
   /// `silo::config::ConfigException` for config value parse errors
   /// (subclass as ConfigValueParseError?).
   virtual void overwriteFromParents(
      const ConsList<std::string>& parents,
      const VerifiedConfigSource& config_source
   ) = 0;

   virtual ~OverwriteFrom() = default;
};
