#include "silo_api/environment_variables.h"

#include <Poco/Environment.h>
#include <spdlog/spdlog.h>
#include <boost/lexical_cast.hpp>

using silo_api::EnvironmentVariables;

std::string EnvironmentVariables::prefixedUppercase(const std::string& key) {
   std::string result;
   for (const char character : key) {
      if (std::isupper(character)) {
         result += '_';
         result += character;
      } else {
         const char char_in_upper_case =
            static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
         result += char_in_upper_case;
      }
   }
   return "SILO_" + result;
}

std::string EnvironmentVariables::configType() const {
   return "environment variable";
}

bool EnvironmentVariables::hasProperty(const std::string& key) const {
   return Poco::Environment::has(prefixedUppercase(key));
}

std::string EnvironmentVariables::getString(const std::string& key) const {
   return Poco::Environment::get(prefixedUppercase(key));
}

int32_t EnvironmentVariables::getInt32(const std::string& key) const {
   return boost::lexical_cast<int32_t>(Poco::Environment::get(prefixedUppercase(key)));
}

uint32_t EnvironmentVariables::getUInt32(const std::string& key) const {
   return boost::lexical_cast<uint32_t>(Poco::Environment::get(prefixedUppercase(key)));
}
