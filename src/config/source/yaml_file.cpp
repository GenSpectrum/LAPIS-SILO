#include "config/source/yaml_file.h"

#include <unordered_set>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/alist.h"

namespace {
std::string configKeyPathToStringYaml(const ConfigKeyPath& config_key_path) {
   return boost::join(config_key_path.path, ".");
}

// Only valid if `isProperSingularValue(node) == true`.
std::string stringFromYaml(const YAML::Node& node) {
   return node.as<std::string>();
}

bool isProperSingularValue(const YAML::Node& node) {
   if (node.IsMap()) {
      SPDLOG_TRACE("isProperSingularValue = false, node is a map");
      return false;
   }
   if (!node.IsDefined()) {
      SPDLOG_TRACE("isProperSingularValue = false, node is not defined");
      return false;
   }
   if (!node.IsScalar()) {
      SPDLOG_TRACE("isProperSingularValue = false, node is not a scalar");
      return false;
   }
   return true;
}

void yamlToPaths(
   const std::string& config_context,
   const YAML::Node& node,
   const ConsList<std::string>& parents,
   std::vector<std::pair<ConfigKeyPath, YAML::Node>>& paths
) {
   if (node.IsMap()) {
      for (const auto& key_value : node) {
         const auto key = key_value.first.as<std::string>();
         // ^ XX what if key is not a string?
         const auto parents2 = parents.cons(key);
         const auto node = key_value.second;
         yamlToPaths(config_context, node, parents2, paths);
      }
   } else {
      ConfigKeyPath path{parents.toVecReverse()};
      if (isProperSingularValue(node)) {
         paths.emplace_back(path, node);
      } else {
         throw silo::config::ConfigException(fmt::format(
            "{}: found non-usable leaf value at nesting {}", config_context, path.toDebugString()
         ));
      }
   }
}

}  // namespace

std::string YamlFile::configContext() const {
   return "YAML file";
}
std::string YamlFile::configKeyPathToString(const ConfigKeyPath& config_key_path) const {
   return configKeyPathToStringYaml(config_key_path);
}

YamlFileContents YamlFile::readFile(const std::filesystem::path& path) {
   auto path_string = path.string();
   SPDLOG_INFO("Reading config from {}", path_string);
   try {
      auto node = YAML::LoadFile(path_string);
      // Collect all paths present
      std::vector<std::pair<ConfigKeyPath, YAML::Node>> paths;
      yamlToPaths(fmt::format("YAML file '{}'", path_string), node, ConsList<std::string>{}, paths);

      return YamlFileContents{path, paths};
   } catch (const YAML::Exception& e) {
      throw std::runtime_error(
         fmt::format("Failed to read YAML config file {}: {}", path_string, e.what())
      );
   }
}

std::string YamlFileContents::configContext() const {
   return fmt::format("YAML file '{}'", content_source.string());
}

std::string YamlFileContents::configKeyPathToString(const ConfigKeyPath& config_key_path) const {
   return configKeyPathToStringYaml(config_key_path);
}

std::unique_ptr<VerifiedConfigSource> YamlFileContents::verify(
   const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
) {
   // No need to stringify and do duplicate check since
   // ConfigKeyPath is actually directly representing YAML paths.

   // Build index of valid paths:
   std::unordered_set<const ConfigKeyPath*> valid_paths{};
   for (const auto& [path, _] : config_structs) {
      valid_paths.insert(&path);
   }

   // Check the ones given, collect erroneous ones in foo.bar syntax
   std::vector<std::string> invalid_config_keys;
   for (const auto& [given_path, _yaml] : paths) {
      if (valid_paths.find(&given_path) == valid_paths.end()) {
         invalid_config_keys.push_back(configKeyPathToString(given_path));
      }
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "keys" : "key";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         configContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   return std::make_unique<VerifiedYamlFile>(VerifiedYamlFile{std::move(*this)});
}

std::string VerifiedYamlFile::configContext() const {
   return base.configContext();
}

std::string VerifiedYamlFile::configKeyPathToString(const ConfigKeyPath& config_key_path) const {
   return base.configKeyPathToString(config_key_path);
}

std::optional<std::string> VerifiedYamlFile::getString(const ConfigKeyPath& config_key_path) const {
   const auto* node = AList<ConfigKeyPath, YAML::Node>(base.paths).get(config_key_path);
   if (!node) {
      return std::nullopt;
   }
   return std::optional{stringFromYaml(*node)};
}

const std::vector<std::string>* VerifiedYamlFile::positionalArgs() const {
   return nullptr;
}
