#pragma once

#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <config/config_backend.h>
#include <config/config_specification.h>

namespace silo::config {

class CommandLineArguments : public ConfigBackend {
   std::vector<std::string> args;

  public:
   explicit CommandLineArguments(std::span<const std::string> args_)
       : args(args_.begin(), args_.end()) {}

   [[nodiscard]] constexpr std::string_view errorContext() const {
      return "command line arguments";
   };

   [[nodiscard]] VerifiedConfigSource verify(const ConfigSpecification& config_specification
   ) const override;

   static std::string configKeyPathToString(const ConfigKeyPath& key_path);

   static AmbiguousConfigKeyPath stringToConfigKeyPath(const std::string& command_line_argument);
};

}  // namespace silo::config
