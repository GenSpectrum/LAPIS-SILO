#include "silo_api/environment_variables.h"

#include <Poco/Environment.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>

using silo_api::EnvironmentVariables;

std::string EnvironmentVariables::prefixedUppercase(const Option& option) {
   std::vector<std::string> result;
   for (const std::string& current_string : option.access_path) {
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
   return "SILO_" + boost::join(result, "_");
}

std::string EnvironmentVariables::configType() const {
   return "environment variable";
}

bool EnvironmentVariables::hasProperty(const Option& option) const {
   return Poco::Environment::has(prefixedUppercase(option));
}

std::optional<std::string> EnvironmentVariables::getString(const Option& option) const {
   if (hasProperty(option)) {
      return Poco::Environment::get(prefixedUppercase(option));
   }
   return std::nullopt;
}
