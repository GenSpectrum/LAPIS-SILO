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
   std::vector<std::string> surplus_args;

   friend CommandLineArguments;

   VerifiedCommandLineArguments(
      CommandLineArguments base_,
      std::unordered_map<std::string, std::string> config_value_by_option_,
      std::vector<std::string> surplus_args_
   )
       : base(std::move(base_)),
         config_value_by_option(std::move(config_value_by_option_)),
         surplus_args(std::move(surplus_args_)) {}

  public:
   [[nodiscard]] std::string configContext() const override;
   [[nodiscard]] std::string configKeyPathToString(const ConfigKeyPath& config_key_path
   ) const override;

   [[nodiscard]] std::optional<std::string> getString(const ConfigKeyPath& option) const override;

   [[nodiscard]] const std::vector<std::string>* positionalArgs() const override;
};
