#pragma once

#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <config/config_source_interface.h>

class VerifiedCommandLineArguments;
class CommandLineArguments : public VerifyConfigSource {
   std::vector<std::string> args;

  public:
   explicit CommandLineArguments(std::span<const std::string> args_) {
      // args(std::ranges::to<decltype(args)>{ args_}) is C++23, thus:
      args.assign(args_.begin(), args_.end());
   }

   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::unique_ptr<VerifiedConfigSource> verify(
      const std::span<const std::pair<ConfigKeyPath, const ConfigValue*>>& config_structs
   ) override;
};

class VerifiedCommandLineArguments : public VerifiedConfigSource {
   CommandLineArguments base;
   std::unordered_map<std::string, std::string> config_value_by_option;
   std::vector<std::string> positional_args;

   friend CommandLineArguments;

   VerifiedCommandLineArguments(
      CommandLineArguments base_,
      std::unordered_map<std::string, std::string> config_value_by_option,
      std::vector<std::string> positional_args
   )
       : base(std::move(base_)),
         config_value_by_option(std::move(config_value_by_option)),
         positional_args(std::move(positional_args)) {}

  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& option) const override;

   [[nodiscard]] const std::vector<std::string>* positionalArgs() const override;
};
