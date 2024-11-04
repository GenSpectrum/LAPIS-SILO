#include "config/source/command_line_arguments.h"

#include <fmt/format.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/panic.h"
#include "silo/config/util/config_exception.h"

std::string asUnixOptionString(const ConfigKeyPath& config_key_path) {
   std::vector<std::string> result{"-"};
   for (const std::string& current_string : config_key_path.path) {
      std::string current_result;
      for (const char character : current_string) {
         if (std::isupper(character)) {
            current_result += '-';
            const char char_in_lower_case =
               static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
            current_result += char_in_lower_case;
         } else {
            current_result += character;
         }
      }
      result.emplace_back(current_result);
   }
   return boost::join(result, "-");
}

std::string CommandLineArguments::configContext() const {
   return "command line arguments";
}

std::string CommandLineArguments::configKeyPathToString(const ConfigKeyPath& config_key_path
) const {
   return asUnixOptionString(config_key_path);
}

std::unique_ptr<VerifiedConfigSource> CommandLineArguments::verify(
   const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
) {
   // Build a map of all the "--foo-bar" style keys (and their
   // values for error reporting)
   auto options = stringifiedKeyToConfigMap(config_structs);

   // Parse the command line, now that we have the option keys
   // and the info about whether they take an argument (any that
   // are not of type bool).

   // E.g. "--api-foo" => "1234"
   std::unordered_map<std::string, std::string> config_value_by_option;
   std::vector<std::string> surplus_args;
   std::vector<std::string> invalid_config_keys;

   for (size_t i = 0; i < args.size(); ++i) {
      const std::string& arg = args[i];
      if (arg.starts_with('-')) {
         if (arg == "--") {
            for (size_t j = i + 1; j < args.size(); ++j) {
               surplus_args.push_back(args[j]);
            }
            break;
         }
         if (auto meta = options.find(arg); meta != options.end()) {
            std::string value;
            if (meta->second->isBool()) {
               value = "1";  // XX currently "true" is not parsed (via boost)
            } else {
               ++i;
               if (i == args.size()) {
                  // VerificationError::ParseError in Rust
                  throw silo::config::ConfigException("missing argument after option " + arg);
               }
               value = args[i];
            }
            // Overwrite value with the last occurrence
            // (i.e. `silo --foo 4 --foo 5` will leave "--foo"
            // => "5" in the map).
            config_value_by_option[arg] = value;
         } else {
            invalid_config_keys.push_back(arg);
         }
      } else {
         surplus_args.push_back(arg);
      }
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "options" : "option";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         configContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   // Need to specify VerifiedCommandLineArguments { } because the
   // constructor is private and std::make_unique foils the friend
   // relationship.
   return std::make_unique<VerifiedCommandLineArguments>(VerifiedCommandLineArguments{
      std::move(*this), std::move(config_value_by_option), std::move(surplus_args)
   });
}

std::string VerifiedCommandLineArguments::configContext() const {
   return base.configContext();
}

std::string VerifiedCommandLineArguments::configKeyPathToString(const ConfigKeyPath& config_key_path
) const {
   return base.configKeyPathToString(config_key_path);
}

std::optional<std::string> VerifiedCommandLineArguments::getString(const ConfigKeyPath& option
) const {
   auto option_string = configKeyPathToString(option);
   if (auto iter = config_value_by_option.find(option_string);
       iter != config_value_by_option.end()) {
      return iter->second;
   }
   return std::nullopt;
}

const std::vector<std::string>* VerifiedCommandLineArguments::positionalArgs() const {
   return &surplus_args;
}
