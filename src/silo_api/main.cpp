#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <variant>

#include "silo/common/overloaded.h"
#include "silo/config/runtime_config.h"
#include "silo_api/logging.h"

int main(int argc, char** argv) {
   setupLogger();

   std::vector<std::string> all_args(argv, argv + argc);

   std::span<const std::string> args(all_args.begin() + 1, all_args.end());

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
