#include "config/backend/command_line_arguments.h"

#include <map>

#include <fmt/format.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

#include "silo/common/panic.h"
#include "silo/config/util/config_exception.h"

namespace silo::config {

std::string CommandLineArguments::configKeyPathToString(const ConfigKeyPath& key_path) {
   std::vector<std::string> result;
   for (const auto& sublevel : key_path.getPath()) {
      for (const std::string& current_string : sublevel) {
         result.push_back(current_string);
      }
   }
   return "--" + boost::join(result, "-");
}

AmbiguousConfigKeyPath CommandLineArguments::stringToConfigKeyPath(
   const std::string& command_line_argument
) {
   if (command_line_argument.size() < 3 || !command_line_argument.starts_with("--")) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid command line option", command_line_argument
      ));
   }

   std::vector<std::string> delimited_strings;
   // Remove the leading dashes and split by '-'
   std::string trimmed = command_line_argument.substr(2);
   std::vector<std::string> tokens;

   boost::split(tokens, trimmed, boost::is_any_of("-"));

   for (const auto& token : tokens) {
      if (token.empty()) {
         throw silo::config::ConfigException(fmt::format(
            "the provided option '{}' is not a valid command line option", command_line_argument
         ));
      }
      delimited_strings.push_back(token);
   }

   auto result = AmbiguousConfigKeyPath::tryFrom(std::move(delimited_strings));
   if (result == std::nullopt) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid command line option", command_line_argument
      ));
   }
   return result.value();
}

namespace {

std::optional<std::string> tryGetAt(const std::vector<std::string>& args, size_t index) {
   if (index < args.size()) {
      return args[index];
   }
   return std::nullopt;
}

class ValueAndConsumedFlag {
  public:
   ConfigValue value;
   bool consumed_next;
};

ValueAndConsumedFlag getValueFromArg(
   const ConfigValueSpecification& value_specification,
   const std::string& arg,
   const std::optional<std::string>& next_arg
) {
   if (value_specification.type == ConfigValueType::BOOL) {
      return {ConfigValue::fromBool(true), false};
   }
   if (!next_arg.has_value()) {
      // VerificationError::ParseError in Rust
      throw silo::config::ConfigException("missing argument after option " + arg);
   }
   return {value_specification.getValueFromString(next_arg.value()), true};
}

}  // namespace

VerifiedConfigSource CommandLineArguments::verify(const ConfigSpecification& config_specification
) const {
   // Parse the command line, now that we have the option keys
   // and the info about whether they take an argument (any that
   // are not of type bool).

   // E.g. "--api-foo" => "1234"
   std::unordered_map<ConfigKeyPath, ConfigValue> config_value_by_option;
   std::vector<std::string> positional_args;
   std::vector<std::string> invalid_config_keys;
   size_t args_index = 0;
   while (args_index < args.size()) {
      const std::string& arg = args[args_index];
      args_index++;
      const std::optional<std::string>& next_arg = tryGetAt(args, args_index);
      if (arg.starts_with('-')) {
         if (arg == "--") {
            break;
         }
         const AmbiguousConfigKeyPath ambiguous_key = stringToConfigKeyPath(arg);
         if (auto value_specification_opt = config_specification.getValueSpecificationFromAmbiguousKey(ambiguous_key)) {
            auto value_specification = value_specification_opt.value();
            const auto value_and_consume = getValueFromArg(value_specification, arg, next_arg);
            if (value_and_consume.consumed_next) {
               ++args_index;
            }
            // Overwrite value with the last occurrence
            // (i.e. `silo --foo 4 --foo 5` will leave "--foo"
            // => "5" in the map).
            config_value_by_option.emplace(value_specification.key, value_and_consume.value);
         } else {
            invalid_config_keys.push_back(arg);
         }
      } else {
         positional_args.push_back(arg);
      }
   }
   while (args_index < args.size()) {
      positional_args.push_back(args[args_index]);
      ++args_index;
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "options" : "option";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         errorContext(),
         keys_or_options,
         boost::join(invalid_config_keys, ", ")
      ));
   }

   // Need to specify VerifiedCommandLineArguments { } because the
   // constructor is private and std::make_unique foils the friend
   // relationship.
   return VerifiedConfigSource{std::move(config_value_by_option)};
}

}  // namespace silo::config
