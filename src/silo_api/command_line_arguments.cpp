#include "silo_api/command_line_arguments.h"

using silo_api::CommandLineArguments;

CommandLineArguments::CommandLineArguments(const Poco::Util::AbstractConfiguration& config)
    : config(config) {}

std::string CommandLineArguments::configType() const {
   return "command line argument";
}

bool CommandLineArguments::hasProperty(const std::string& key) const {
   return config.hasProperty(key);
}

std::string CommandLineArguments::getString(const std::string& key) const {
   return config.getString(key);
}

int32_t CommandLineArguments::getInt32(const std::string& key) const {
   return config.getInt(key);
}

uint32_t CommandLineArguments::getUInt32(const std::string& key) const {
   return config.getUInt(key);
}
