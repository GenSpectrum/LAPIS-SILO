#include "silo_api/command_line_arguments.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/config/util/config_exception.h"

namespace silo_api {

std::string CommandLineArguments::asUnixOptionString(
   const silo::config::AbstractConfigSource::Option& option
) {
   std::vector<std::string> result;
   for (const std::string& current_string : option.access_path) {
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

CommandLineArguments::CommandLineArguments(const Poco::Util::AbstractConfiguration& config)
    : config(config) {}

std::string CommandLineArguments::configType() const {
   return "command line argument";
}

bool CommandLineArguments::hasProperty(const Option& option) const {
   return config.hasProperty(asUnixOptionString(option));
}

std::optional<std::string> CommandLineArguments::getString(const Option& option) const {
   if (hasProperty(option)) {
      return config.getString(asUnixOptionString(option));
   }
   return std::nullopt;
}

}  // namespace silo_api
