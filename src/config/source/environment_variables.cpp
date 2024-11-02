#include "config/source/environment_variables.h"

#include <unistd.h>

#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/alist.h"

const char* config_context = "environment variables";
const char* env_var_prefix = "SILO_";

namespace {
std::string prefixedUppercase(const ConfigKeyPath& option) {
   std::vector<std::string> result;
   for (const std::string& current_string : option.path) {
      std::string current_result;
      for (const char character : current_string) {
         if (std::isupper(character)) {
            current_result += '_';
            current_result += character;
         } else {
            const char char_in_upper_case =
               static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
            current_result += char_in_upper_case;
         }
      }
      result.emplace_back(current_result);
   }
   return env_var_prefix + boost::join(result, "_");
}
}  // namespace

std::string EnvironmentVariables::configContext() const {
   return config_context;
}

[[nodiscard]] std::string EnvironmentVariables::configKeyPathToString(
   const ConfigKeyPath& config_key_path
) const {
   return prefixedUppercase(config_key_path);
}

std::unique_ptr<DecodedEnvironmentVariables> EnvironmentVariables::parse(const char* const* envp) {
   std::vector<std::pair<std::string, std::string>> alist;
   for (const char* const* current_envp = envp; *current_envp != nullptr; current_envp++) {
      const char* env = *current_envp;
      for (size_t i = 0; env[i] != 0; i++) {
         if (env[i] == '=') {
            std::string key{env, i};
            if (key.starts_with(env_var_prefix)) {
               std::string val{env + i + 1};
               alist.emplace_back(key, val);
            }
         }
      }
   }
   return std::make_unique<DecodedEnvironmentVariables>(DecodedEnvironmentVariables{std::move(alist)
   });
}

[[nodiscard]] std::unique_ptr<VerifiedConfigSource> DecodedEnvironmentVariables::verify(
   const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
) {
   // Build a map of all the "SILO_FOO_BAR" style keys (and their
   // values for error reporting)
   auto options = stringifiedKeyToConfigMap(config_structs);

   std::vector<std::string> invalid_config_keys;
   for (const auto& [k, _v] : alist) {
      if (auto meta = options.find(k); meta == options.end()) {
         if (k == "SILO_PANIC") {
            SPDLOG_TRACE("allowing env variable {} which is independent of the config system", k);
         } else {
            invalid_config_keys.push_back(k);
         }
      }
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "variables" : "variable";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         configContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   return std::make_unique<VerifiedEnvironmentVariables>(
      VerifiedEnvironmentVariables{std::move(*this)}
   );
}

std::string DecodedEnvironmentVariables::configContext() const {
   return config_context;
}

[[nodiscard]] std::string DecodedEnvironmentVariables::configKeyPathToString(
   const ConfigKeyPath& config_key_path
) const {
   return prefixedUppercase(config_key_path);
}

std::string VerifiedEnvironmentVariables::configContext() const {
   return config_context;
}

[[nodiscard]] std::string VerifiedEnvironmentVariables::configKeyPathToString(
   const ConfigKeyPath& config_key_path
) const {
   return prefixedUppercase(config_key_path);
}

std::optional<std::string> VerifiedEnvironmentVariables::getString(
   const ConfigKeyPath& config_key_path
) const {
   auto key = configKeyPathToString(config_key_path);
   const auto* val = AList(base.alist).get(key);
   if (!val) {
      return std::nullopt;
   }
   return std::optional{*val};
}

const std::vector<std::string>* VerifiedEnvironmentVariables::positionalArgs() const {
   return nullptr;
}
