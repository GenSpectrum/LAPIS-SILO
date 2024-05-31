#include "silo/config/util/abstract_config_source.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lexical_cast/bad_lexical_cast.hpp>

#include "silo/config/util/config_exception.h"

namespace silo::config {

std::string AbstractConfigSource::Option::toString() const {
   return boost::join(access_path, ".");
}

std::optional<int32_t> AbstractConfigSource::getInt32(const Option& option) const {
   const auto string_value = getString(option);
   if (string_value == std::nullopt) {
      return std::nullopt;
   }
   try {
      return boost::lexical_cast<int32_t>(*string_value);
   } catch (boost::bad_lexical_cast&) {
      const std::string error_message = fmt::format(
         "Could not cast the value '{}' from the {} option '{}' to a 32-bit signed integer.",
         *string_value,
         configType(),
         option.toString()
      );
      SPDLOG_ERROR(error_message);
      throw ConfigException(error_message);
   }
}

std::optional<uint16_t> AbstractConfigSource::getUInt16(const Option& option) const {
   const auto string_value = getString(option);
   if (string_value == std::nullopt) {
      return std::nullopt;
   }
   try {
      return boost::lexical_cast<uint16_t>(*string_value);
   } catch (boost::bad_lexical_cast&) {
      const std::string error_message = fmt::format(
         "Could not cast the value '{}' from the {} option '{}' to a 16-bit unsigned integer.",
         *string_value,
         configType(),
         option.toString()
      );
      SPDLOG_ERROR(error_message);
      throw ConfigException(error_message);
   }
}

std::optional<uint32_t> AbstractConfigSource::getUInt32(const Option& option) const {
   const auto string_value = getString(option);
   if (string_value == std::nullopt) {
      return std::nullopt;
   }
   try {
      return boost::lexical_cast<uint32_t>(*string_value);
   } catch (boost::bad_lexical_cast&) {
      const std::string error_message = fmt::format(
         "Could not cast the value '{}' from the {} option '{}' to a 32-bit unsigned integer.",
         *string_value,
         configType(),
         option.toString()
      );
      SPDLOG_ERROR(error_message);
      throw ConfigException(error_message);
   }
}

}  // namespace silo::config
