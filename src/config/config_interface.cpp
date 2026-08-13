#include "config/config_interface.h"

namespace rhydb::config {

std::optional<std::filesystem::path> getConfigFilePath(
   const rhydb::config::ConfigKeyPath& config_key_path,
   const VerifiedCommandLineArguments& cmd_source,
   const VerifiedConfigAttributes& env_source
) {
   if (auto path = cmd_source.getPath(config_key_path)) {
      return path;
   }
   if (auto path = env_source.getPath(config_key_path)) {
      return path;
   }
   return std::nullopt;
}

}  // namespace rhydb::config
