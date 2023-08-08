#ifndef SILO_RUNTIME_CONFIG_H
#define SILO_RUNTIME_CONFIG_H

#include <filesystem>
#include <optional>

namespace silo_api {

struct RuntimeConfig {
   std::optional<std::filesystem::path> data_directory;

   static RuntimeConfig readFromFile(const std::filesystem::path& config_path);
};

}  // namespace silo_api

#endif  // SILO_RUNTIME_CONFIG_H
