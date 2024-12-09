#pragma once

#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <config/config_source_interface.h>
#include <config/config_specification.h>

namespace silo::config {

class CommandLineArguments {
   std::vector<std::string> args;

   [[nodiscard]] inline std::string debugContext() const { return "command line arguments"; };

  public:
   explicit CommandLineArguments(std::span<const std::string> args_)
       : args(args_.begin(), args_.end()) {}

   using VerifiedType = VerifiedCommandLineArguments;
   [[nodiscard]] VerifiedType verify(const ConfigSpecification& config_specification) const;

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static AmbiguousConfigKeyPath stringToConfigKeyPath(const std::string& option);
};

}  // namespace silo::config
