#pragma once

#include <iostream>
#include <optional>
#include <span>
#include <variant>

#include "config/config_source_interface.h"
#include "config/config_specification.h"
#include "config/source/command_line_arguments.h"
#include "config/source/environment_variables.h"
#include "config/source/yaml_file.h"
#include "silo/common/cons_list.h"
#include "silo/common/overloaded.h"
#include "silo/config/util/config_exception.h"

namespace silo::config {

/// For config structs (containing help and possibly config file paths):
/// We use a concept instead of virtual method overrides.
/// This is because we want to call the virtual method overwriteFrom for the default values when
/// constructing a Config. Instead, making the constructor private and instead creating a factory
/// method would also be possible. Here, a concept works great, because the only usage of the
/// interface uses a template anyways, due to the different return types (RuntimeConfig vs.
/// PreprocessingConfig), whose easily accessible structure should remain.
template <typename C>
concept Config = requires(C c, const C cc, const VerifiedConfigSource& config_source) {
   /// Get the specification (ConfigSpecification) for this kind (C)
   /// of config.
   { C::getConfigSpecification() } -> std::same_as<ConfigSpecification>;

   /// Whether the user gave the --help option or environment
   /// variable equivalent.
   { cc.asksForHelp() } -> std::same_as<bool>;

   /// Optional config file that the user gave (or that is provided
   /// by the type via its defaults) that should be loaded.
   { cc.configPath() } -> std::same_as<std::optional<std::filesystem::path>>;

   /// Overwrite the fields of an instance of the target type; done
   /// that way so that multiple kinds of config sources can shadow
   /// each other's values by application in sequence. Does not throw
   /// exceptions, except overwriteFrom can call SILO_PANIC when there
   /// is an inconsistency (bug) between ConfigSpecification and
   /// overwriteFrom implementation.
   { c.overwriteFrom(config_source) } -> std::same_as<void>;

   /// Validation / Sanity checks about the values of this config
   { c.validate() } -> std::same_as<void>;
};

/// In case of error, returns the exit code that the caller should
/// pass to exit(): 0 if the user gave --help, 1 in case of erroneous
/// usage (the error is already printed in that case).
template <Config C>
std::variant<C, int32_t> getConfig(std::span<const std::string> cmd) {
   const auto config_specification = C::getConfigSpecification();
   try {
      auto cmd_source = CommandLineArguments{cmd}.verify(config_specification);
      if (!cmd_source.positional_arguments.empty()) {
         throw silo::config::ConfigException{"SILO does not expect positional arguments"};
      }

      C config;

      // First, only check command line arguments, for "--help"; avoid
      // potential errors from env processing, and we don't have the
      // path to the config file yet.
      config = {};
      config.overwriteFrom(cmd_source);
      if (config.asksForHelp()) {
         std::cout << config_specification.helpText() << "\n" << std::flush;
         return 0;
      }

      auto env_source =
         EnvironmentVariables::decodeEnvironmentVariables().verify(config_specification);
      // Restart from scratch, because the values from cmd_source need
      // to shadow the ones from env_source.
      config = {};
      config.overwriteFrom(env_source);
      config.overwriteFrom(cmd_source);

      // Was a config file given as an argument or by environment variable?
      auto config_path = config.configPath();
      if (config_path.has_value()) {
         auto file_source = YamlConfig::readFile(*config_path).verify(config_specification);
         // Now read again with the file first:
         config = {};
         config.overwriteFrom(file_source);
         config.overwriteFrom(env_source);
         config.overwriteFrom(cmd_source);
         // (The config file might specify --help, too, but we ignore that.)
      }
      config.validate();
      return std::move(config);
   } catch (const silo::config::ConfigException& e) {
      std::cerr << fmt::format(
                      "Usage error: {}.\n\nRun with the --help option for help.\n", e.what()
                   )
                << std::flush;
      return 1;
   }
}

}  // namespace silo::config
