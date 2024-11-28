#include "config/source/yaml_file.h"

#include <fstream>
#include <istream>
#include <unordered_set>

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/alist.h"
#include "silo/common/fmt_formatters.h"
#include "silo/config/util/config_exception.h"

using silo::config::ConfigKeyPath;

namespace {

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

std::vector<std::string> splitByDot(const std::string& str) {
   std::vector<std::string> result;
   std::stringstream buffer(str);
   std::string token;

   while (std::getline(buffer, token, '.')) {
      result.push_back(token);
   }

   return result;
}

std::vector<std::string> splitCamelCase(const std::string& camelCaseString) {
   std::vector<std::string> result;
   std::string current;

   for (const char character : camelCaseString) {
      if (std::isupper(character)) {
         // If current is not empty, push it to the result
         if (!current.empty()) {
            result.push_back(current);
            current.clear();
         }
         // Add the lowercase version of the uppercase char as the start of a new substring
         current += static_cast<char>(std::tolower(character));
      } else {
         // Append lowercase or non-uppercase char to current
         current += character;
      }
   }
   // Push the last accumulated string to result
   if (!current.empty()) {
      result.push_back(current);
   }

   return result;
}

std::string joinCamelCase(const std::vector<std::string>& words) {
   std::string camel_case_string;

   for (size_t i = 0; i < words.size(); ++i) {
      if (i == 0) {
         // Add the first word as is (lowercase)
         camel_case_string += words[i];
      } else {
         // Capitalize the first character of subsequent words and append them
         std::string word = words[i];
         if (!word.empty()) {
            word[0] = static_cast<char>(std::toupper(word[0]));
            camel_case_string += word;
         }
      }
   }

   return camel_case_string;
}

// NOLINTNEXTLINE(misc-no-recursion)
void yamlToPaths(
   const std::string& config_context,
   const YAML::Node& node,
   const ConsList<std::vector<std::string>>& parents,
   std::unordered_map<ConfigKeyPath, YAML::Node>& paths
) {
   if (node.IsMap()) {
      for (const auto& key_value : node) {
         const auto key = key_value.first.as<std::string>();
         // ^ XX what if key is not a string?
         const auto parents2 = parents.cons(splitCamelCase(key));
         const auto child_node = key_value.second;
         yamlToPaths(config_context, child_node, parents2, paths);
      }
   } else {
      auto path = ConfigKeyPath::tryFrom(parents.toVecReverse());
      if (!path) {
         // transform the ConsList parents into a debug string by reversing it,
         // transforming the elements into camel-case and joining them with a '.'
         const std::string debug_string_parents = parents.foldRight(
            std::string{},
            [](const std::vector<std::string>& rest_of_parents, const std::string& acc) {
               return acc + "." + joinCamelCase(rest_of_parents);
            }
         );
         throw silo::config::ConfigException(
            fmt::format("{}: found invalid key", debug_string_parents)
         );
      }
      if (isProperSingularValue(node)) {
         paths.emplace(path.value(), node);
      } else {
         throw silo::config::ConfigException(fmt::format(
            "{}: found non-usable leaf value at nesting {}",
            config_context,
            path.value().toDebugString()
         ));
      }
   }
}

}  // namespace

namespace silo::config {

std::string YamlConfig::configKeyPathToString(const ConfigKeyPath& config_key_path) {
   std::vector<std::string> camel_case_strings;
   for (const auto& list : config_key_path.getPath()) {
      camel_case_strings.emplace_back(joinCamelCase(list));
   }
   return boost::join(camel_case_strings, ".");
}

ConfigKeyPath YamlConfig::stringToConfigKeyPath(const std::string& key_path_string) {
   const std::vector<std::string> camel_case_strings = splitByDot(key_path_string);
   std::vector<std::vector<std::string>> result;
   std::transform(
      camel_case_strings.begin(),
      camel_case_strings.end(),
      std::back_inserter(result),
      splitCamelCase
   );
   return ConfigKeyPath::tryFrom(result).value();
}

YamlConfig YamlConfig::fromYAML(const std::string& error_context, const std::string& yaml_string) {
   try {
      const YAML::Node node = YAML::Load(yaml_string);

      // Collect all paths present
      std::unordered_map<ConfigKeyPath, YAML::Node> paths;
      yamlToPaths(error_context, node, ConsList<std::vector<std::string>>{}, paths);

      return YamlConfig{error_context, paths};
   } catch (const YAML::ParserException& parser_exception) {
      throw std::runtime_error(
         fmt::format("{} does not contain valid YAML: {}", error_context, parser_exception.what())
      );
   }
}

YamlConfig YamlConfig::readFile(const std::filesystem::path& path) {
   std::ifstream file(path, std::ios::in | std::ios::binary);
   if (file.fail()) {
      throw std::runtime_error(fmt::format("Could not open the YAML file: '{}'", path));
   }

   std::ostringstream contents;
   if (file.peek() != std::ifstream::traits_type::eof()) {
      contents << file.rdbuf();
   }
   if (contents.fail()) {
      throw std::runtime_error(fmt::format("Error when reading the YAML file: '{}'", path));
   }

   return fromYAML(fmt::format("file: '{}'", path.string()), contents.str());
}

std::string YamlConfig::errorContext() const {
   return fmt::format("YAML file '{}'", error_context);
}

namespace {
ConfigValue yamlNodeToConfigValue(
   const ConfigValueSpecification& value_specification,
   const YAML::Node& yaml
) {
   switch (value_specification.type) {
      case ConfigValueType::STRING:
         return value_specification.createValue(yaml.as<std::string>());
      case ConfigValueType::PATH:
         return value_specification.createValue({std::filesystem::path{yaml.as<std::string>()}});
      case ConfigValueType::INT32:
         return value_specification.createValue(yaml.as<int32_t>());
      case ConfigValueType::UINT32:
         return value_specification.createValue(yaml.as<uint32_t>());
      case ConfigValueType::UINT16:
         return value_specification.createValue(yaml.as<uint16_t>());
      case ConfigValueType::BOOL:
         return value_specification.createValue(yaml.as<bool>());
   }
   SILO_UNREACHABLE();
}
}  // namespace

VerifiedConfigSource YamlConfig::verify(const ConfigSpecification& config_specification) const {
   // No need to stringify and do duplicate check since
   // ConfigKeyPath is actually directly representing YAML paths.

   // Check the ones given, collect erroneous ones in foo.bar syntax
   std::vector<std::string> invalid_config_keys;
   std::unordered_map<ConfigKeyPath, ConfigValue> provided_config_values;
   for (const auto& [key, yaml] : getYamlFields()) {
      auto value_specification = config_specification.getValueSpecification(key);
      if (!value_specification.has_value()) {
         invalid_config_keys.push_back(configKeyPathToString(key));
      } else {
         const ConfigValue value = yamlNodeToConfigValue(value_specification.value(), yaml);
         provided_config_values.emplace(key, value);
      }
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "keys" : "key";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         errorContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   return VerifiedConfigSource{provided_config_values};
}

const std::unordered_map<ConfigKeyPath, YAML::Node>& YamlConfig::getYamlFields() const {
   return yaml_fields;
}

}  // namespace silo::config
