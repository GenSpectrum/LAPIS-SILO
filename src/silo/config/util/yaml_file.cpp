#include "silo/config/util/yaml_file.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>

using silo::config::YamlFile;

YamlFile::YamlFile(const std::filesystem::path& filename)
    : filename(filename) {
   SPDLOG_INFO("Reading config from {}", filename.string());
   try {
      node = YAML::LoadFile(filename.string());
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         fmt::format("Failed to read preprocessing config from {}: {}", filename.string(), e.what())
      );
   }
}

std::string YamlFile::configType() const {
   return fmt::format("yaml config file '{}'", filename.string());
}

bool YamlFile::hasProperty(const Option& option) const {
   YAML::Node current = Clone(node);
   for (const auto& access : option.access_path) {
      if (!current.IsDefined() || !current.IsMap()) {
         return false;
      }
      current = current[access];
   }
   return current.IsDefined();
}

std::optional<std::string> YamlFile::getString(const Option& option) const {
   YAML::Node current = Clone(node);
   for (const auto& access : option.access_path) {
      if (!current.IsDefined() || !current.IsMap()) {
         return std::nullopt;
      }
      current = current[access];
   }
   if (!current.IsDefined() || !current.IsScalar()) {
      return std::nullopt;
   }
   return current.as<std::string>();
}
