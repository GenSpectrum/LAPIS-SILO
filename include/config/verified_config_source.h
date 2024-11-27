#pragma once

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "config/config_key_path.h"
#include "config/config_value.h"

namespace silo::config {

/// A VerifiedConfigSource is providing I/O- and key error free (but
/// not necessarily value-error free) access to a set of configuration
/// data.
class VerifiedConfigSource {
  public:
   std::unordered_map<ConfigKeyPath, ConfigValue> config_values;

   /// Retrieve a config value for the given key as a string
   /// (potentially converting other value types). (Explicitly
   /// getting as a string is necessary for YAML, where the YAML
   /// parser already has some typed representations but not
   /// necessarily those we need. (Todo: this is a hack, improve.))
   /// This returns an option since even though invalid options are
   /// not present in self, the given option may also not be present.
   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& config_key_path) const;

   [[nodiscard]] std::optional<std::filesystem::path> getPath(const ConfigKeyPath& config_key_path
   ) const;

   [[nodiscard]] std::optional<int32_t> getInt32(const ConfigKeyPath& config_key_path) const;

   [[nodiscard]] std::optional<uint32_t> getUint32(const ConfigKeyPath& config_key_path) const;

   [[nodiscard]] std::optional<uint16_t> getUint16(const ConfigKeyPath& config_key_path) const;

   [[nodiscard]] std::optional<double> getFloat(const ConfigKeyPath& config_key_path) const;

   [[nodiscard]] std::optional<bool> getBool(const ConfigKeyPath& config_key_path) const;
};

}  // namespace silo::config
