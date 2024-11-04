#include <filesystem>
#include <iostream>
#include <variant>

#include <boost/algorithm/string/join.hpp>

#include "silo/common/overloaded.h"
#include "silo/config/runtime_config.h"
#include "silo_api/logging.h"

int main(int argc, char** argv) {
   setupLogger();

   std::vector<std::string> all_args(argv, argv + argc);

   std::filesystem::path program_path{all_args[0]};

   std::string program_name = program_path.filename();

   std::span<const std::string> args(all_args.begin() + 1, all_args.end());

   int mode;
   if (program_name == "siloPreprocessor") {
      mode = 0;
   } else if (program_name == "siloServer") {
      mode = 1;
   } else if (!args.empty()) {
      const std::string& mode_argument = args[0];
      args = {args.begin() + 1, args.end()};
      if (mode_argument == "preprocess") {
         mode = 0;
      } else if (mode_argument == "api") {
         mode = 1;
      } else {
         std::cerr << program_name
                   << ": need either 'preprocess' or 'api' as the first program argument, got '"
                   << mode_argument << "'\n";
         return 1;
      }
   } else {
      std::cerr << program_name
                << ": need either 'preprocess' or 'api' as the first program argument\n";
      return 1;
   }

   if (mode == 0) {
      // XXX preprocessing config
   } else {
      return std::visit(
         overloaded{
            [&](const silo::config::RuntimeConfig& runtime_config) {
               SPDLOG_TRACE("runtime_config = {}", runtime_config);
               return 0;
            },
            [&](int32_t exit_code) { return exit_code; }
         },
         getConfig<silo::config::RuntimeConfig>(args, silo::config::RUNTIME_CONFIG_METADATA)
      );
   }
}
