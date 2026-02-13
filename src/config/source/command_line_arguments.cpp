#include "config/source/command_line_arguments.h"

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include "config/config_exception.h"

namespace silo::config {

std::string CommandLineArguments::configKeyPathToString(const ConfigKeyPath& key_path) {
   std::vector<std::string> result;
   for (const auto& sublevel : key_path.getPath()) {
      for (const std::string& current_string : sublevel) {
         result.push_back(current_string);
      }
   }
   return fmt::format("--{}", fmt::join(result, "-"));
}

AmbiguousConfigKeyPath CommandLineArguments::stringToConfigKeyPath(const std::string& option) {
   if (option.size() < 3 || !option.starts_with("--")) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid command line option"
         " as silo currently only accepts long-form options starting with '--'",
         option
      ));
   }
   std::string trimmed = option.substr(2);

   std::vector<std::string> delimited_strings;
   boost::split(delimited_strings, trimmed, boost::is_any_of("-"));

   if (std::ranges::any_of(delimited_strings, [](const auto& str) { return str.empty(); })) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid command line option"
         " because it contains an empty string segment between '-'",
         option
      ));
   }

   auto result = AmbiguousConfigKeyPath::tryFrom(std::move(delimited_strings));
   if (result == std::nullopt) {
      throw silo::config::ConfigException(fmt::format(
         "the provided option '{}' is not a valid command line option. The string after -- should "
         "be '-' delimited and lower-case",
         option
      ));
   }
   return result.value();
}

namespace {

// Split `--foo-bar=baz` -> `{"--foo-bar", "baz"}`, `--foo-bar` ->
// `{"--foo-bar", nullopt}`.
std::tuple<std::string, std::optional<std::string>> splitOption(const std::string& arg) {
   const std::size_t pos = arg.find('=');
   if (pos != std::string::npos) {
      return {arg.substr(0, pos), arg.substr(pos + 1)};
   }
   return {arg, std::nullopt};
}

// Parse an option's value, taking the next argument if needed.
std::tuple<ConfigValue, std::span<const std::string>> parseValueFromArg(
   const ConfigAttributeSpecification& attribute_spec,
   const std::string& arg,
   const std::optional<std::string>& opt_value_string,
   std::span<const std::string> remaining_args
) {
   if (attribute_spec.type == ConfigValueType::BOOL) {
      if (opt_value_string.has_value()) {
         throw silo::config::ConfigException("'=' not acceptable for boolean option: " + arg);
      }
      return {ConfigValue::fromBool(true), remaining_args};
   }
   std::string value_string;
   if (opt_value_string.has_value()) {
      value_string = opt_value_string.value();
   } else {
      if (remaining_args.empty()) {
         throw silo::config::ConfigException("missing argument after option " + arg);
      }
      value_string = remaining_args[0];
      remaining_args = remaining_args.subspan(1);
   }
   return {attribute_spec.parseValueFromString(value_string), remaining_args};
}

}  // namespace

VerifiedCommandLineArguments CommandLineArguments::verify(
   const ConfigSpecification& config_specification
) const {
   // Now, given config_specification (and thus which options are
   // boolean and which take arguments), we can parse the command
   // line.

   // E.g. "--api-foo" => "1234" or "--api-foo=1234"
   std::unordered_map<ConfigKeyPath, ConfigValue> config_value_by_option;
   std::vector<std::string> positional_args;
   std::vector<std::string> invalid_config_keys;
   std::span<const std::string> remaining_args{args.data(), args.size()};
   while (!remaining_args.empty()) {
      const std::string& arg = remaining_args[0];
      remaining_args = remaining_args.subspan(1);
      if (arg.starts_with('-')) {
         if (arg == "--") {
            std::ranges::copy(remaining_args, std::back_inserter(positional_args));
            break;
         }
         if (arg == "-h" || arg == "--help") {
            return VerifiedCommandLineArguments::askingForHelp();
         }
         const auto [option, opt_value_string] = splitOption(arg);
         const auto ambiguous_key = stringToConfigKeyPath(option);
         if (auto opt =
                config_specification.getAttributeSpecificationFromAmbiguousKey(ambiguous_key)) {
            const ConfigAttributeSpecification attribute_spec = std::move(opt.value());
            const auto [value, new_remaining_args] =
               parseValueFromArg(attribute_spec, arg, opt_value_string, remaining_args);
            remaining_args = new_remaining_args;
            // Overwrite value with the last occurrence
            // (i.e. `silo --foo 4 --foo 5` will leave "--foo"
            // => "5" in the map).
            config_value_by_option.emplace(attribute_spec.key, value);
         } else {
            invalid_config_keys.push_back(option);
         }
      } else {
         positional_args.push_back(arg);
      }
   }

   if (!invalid_config_keys.empty()) {
      const char* keys_or_options = (invalid_config_keys.size() >= 2) ? "options" : "option";
      throw silo::config::ConfigException(fmt::format(
         "in {}: unknown {} {}",
         debugContext(),
         keys_or_options,
         fmt::join(invalid_config_keys, ", ")
      ));
   }

   return VerifiedCommandLineArguments::fromConfigValuesAndPositionalArguments(
      std::move(config_value_by_option), std::move(positional_args)
   );
}

}  // namespace silo::config
