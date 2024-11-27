#include "config/backend/environment_variables.h"

#include <cstddef>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/alist.h"

constexpr std::string_view ENV_VAR_PREFIX = "SILO_";

namespace {

std::string toLowerCase(std::string input) {
   std::string result;
   std::ranges::transform(input, std::back_inserter(result), ::tolower);
   return result;
}

}  // namespace

namespace silo::config {

EnvironmentVariables EnvironmentVariables::decodeEnvironmentVariables(const char* const* envp) {
   std::vector<std::pair<std::string, std::string>> alist;
   for (const char* const* current_envp = envp; *current_envp != nullptr; current_envp++) {
      const char* env = *current_envp;
      for (size_t i = 0; env[i] != 0; i++) {
         if (env[i] == '=') {
            const std::string key{env, i};
            if (key.starts_with(ENV_VAR_PREFIX)) {
               const std::string val{env + i + 1};
               alist.emplace_back(key, val);
            }
         }
      }
   }
   return EnvironmentVariables{std::move(alist)};
}

[[nodiscard]] std::string EnvironmentVariables::configKeyPathToString(
   const ConfigKeyPath& config_key_path
) {
   std::vector<std::string> result;
   for (const auto& sublevel : config_key_path.getPath()) {
      for (const std::string& current_string : sublevel) {
         std::string current_string_all_uppercase;
         std::ranges::transform(
            current_string,
            std::back_inserter(current_string_all_uppercase),
            [](unsigned char character) { return std::toupper(character); }
         );
         result.push_back(current_string_all_uppercase);
      }
   }
   return fmt::format("{}{}", ENV_VAR_PREFIX, boost::join(result, "_"));
}

AmbiguousConfigKeyPath EnvironmentVariables::stringToConfigKeyPath(
   const std::string& key_path_string
) {
   std::vector<std::string> delimited_strings;

   if (!key_path_string.starts_with(ENV_VAR_PREFIX)) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid environment variable option", key_path_string
      ));
   }

   // Remove the prefix
   const std::string trimmed = key_path_string.substr(ENV_VAR_PREFIX.size());

   // Split by '_'
   std::stringstream buffer(trimmed);
   std::string token;
   while (std::getline(buffer, token, '_')) {
      delimited_strings.push_back(toLowerCase(token));
   }

   auto result = AmbiguousConfigKeyPath::tryFrom(std::move(delimited_strings));
   if (result == std::nullopt) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid environment variable option", key_path_string
      ));
   }
   return result.value();
}

[[nodiscard]] VerifiedConfigSource EnvironmentVariables::verify(
   const ConfigSpecification& config_specification
) const {
   std::unordered_map<ConfigKeyPath, ConfigValue> config_values;
   std::vector<std::string> invalid_config_keys;
   for (const auto& [key_string, value_string] : alist) {
      auto ambiguous_key = EnvironmentVariables::stringToConfigKeyPath(key_string);
      auto value_specification_opt =
         config_specification.getValueSpecificationFromAmbiguousKey(ambiguous_key);
      if (value_specification_opt.has_value()) {
         auto value_specification = value_specification_opt.value();
         const ConfigValue value = value_specification.getValueFromString(value_string);
         config_values.emplace(value_specification.key, value);
      } else {
         if (key_string == "SILO_PANIC") {
            SPDLOG_TRACE(
               "allowing env variable {} which is independent of the config system", key_string
            );
         } else {
            invalid_config_keys.push_back(key_string);
         }
      }
   }

   if (!invalid_config_keys.empty()) {
      const std::string_view keys_or_options =
         (invalid_config_keys.size() >= 2) ? "variables" : "variable";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         errorContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   return VerifiedConfigSource{config_values};
}

}  // namespace silo::config
