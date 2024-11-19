#pragma once

#include <iostream>
#include <optional>
#include <span>
#include <variant>

#include "config/config_metadata.h"
#include "config/overwrite_from_interface.h"
#include "config/source/command_line_arguments.h"
#include "config/source/environment_variables.h"
#include "config/source/yaml_file.h"
#include "silo/common/cons_list.h"
#include "silo/common/overloaded.h"

/// For top-level config structs (containing help and possibly config
/// file paths):
class ToplevelConfig : public OverwriteFrom {
  public:
   /// Whether the user gave the --help option or environment
   /// variable equivalent.
   [[nodiscard]] virtual bool asksForHelp() const = 0;

   void overwriteFrom(const VerifiedConfigSource& config_source);

   /// Optional config file that the user gave (or that is provided
   /// by the type via its defaults) that should be loaded.
   [[nodiscard]] virtual std::optional<std::filesystem::path> configPath() const = 0;
};

template <typename C /* : Default + OverwriteFrom + ToplevelConfig + Debug */>
std::optional<C> rawGetConfig(std::span<const std::string> cmd, const ConfigStruct& config_struct) {
   auto config_values = config_struct.configValues();
   auto env_source = EnvironmentVariables::parse()->verify(config_values);
   auto cmd_source = CommandLineArguments{cmd}.verify(config_values);

   C config;

   // First, only check command line arguments, for "--help"; avoid
   // potential errors from env processing, and we don't have the
   // path to the config file yet. Since we're only interested in the
   // help option, there's no need to read config_struct first, OK?
   config = {};
   config.overwriteFrom(*cmd_source);
   if (config.asksForHelp()) {
      return std::nullopt;
   }

   // Then process env and cmd, to get to the config file
   // path. Re-initialize since env must be processed for cmd.
   config = {};
   config.overwriteFrom(config_struct);
   config.overwriteFrom(*env_source);
   config.overwriteFrom(*cmd_source);
   // Would anyone request help via SILO_ENV=true? Well, allow it:
   if (config.asksForHelp()) {
      return std::nullopt;
   }

   auto config_path = config.configPath();
   if (config_path.has_value()) {
      auto file_source = YamlFile::readFile(*config_path).verify(config_values);
      // Now read again with the file first:
      config = {};
      config.overwriteFrom(config_struct);
      config.overwriteFrom(*file_source);
      config.overwriteFrom(*env_source);
      config.overwriteFrom(*cmd_source);
      // (The config file might specify --help, too, but we ignore
      // that.)
   }
   return std::optional{config};
}

/// In case of error, returns the exit code that the caller should
/// pass to exit(): 0 if the user gave --help, 1 in case of erroneous
/// usage (the error is already printed in that case).
template <typename C /* : Default + OverwriteFrom + ToplevelConfig + Debug */>
std::variant<C, int32_t> getConfig(
   std::span<const std::string> cmd,
   const ConfigStruct& config_struct
) {
   try {
      auto config = rawGetConfig<C>(cmd, config_struct);
      if (!config.has_value()) {
         // User requested help
         std::cout << config_struct.helpText() << "\n" << std::flush;
         return 0;
      }
      return std::move(*config);
   } catch (const silo::config::ConfigException& e) {
      std::cerr << fmt::format(
                      "Usage error: {}.\n\nRun with the --help option for help.\n", e.what()
                   )
                << std::flush;
      return 1;
   }
}
