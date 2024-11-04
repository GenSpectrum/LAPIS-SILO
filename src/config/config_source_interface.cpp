#include "config/config_source_interface.h"

std::unordered_map<std::string, const ConfigValue*> VerifyConfigSource::stringifiedKeyToConfigMap(
   const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
) const {
   std::unordered_map<std::string, ConfigKeyPath> seen_paths;

   std::unordered_map<std::string, const ConfigValue*> options;
   for (const auto& [path, config_value] : config_structs) {
      std::string config_key = configKeyPathToString(path);
      auto old = seen_paths.find(config_key);
      if (old != seen_paths.end()) {
         auto msg = fmt::format(
            "multiple occurrence of the same option key {}: "
            "first {} and now {}",
            config_key,
            path.toDebugString(),
            old->second.toDebugString()
         );
         throw std::runtime_error(msg);
      }
      seen_paths[config_key] = path;
      options[config_key] = config_value;
   }

   return options;
}
