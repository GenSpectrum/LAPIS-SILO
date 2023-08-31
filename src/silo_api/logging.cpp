#include "silo_api/logging.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <ratio>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "silo/common/log.h"

static const int MAX_FILES_7 = 7;
static const int AT_MIDNIGHT = 0;
static const int AT_0_MINUTES = 0;
static const bool DONT_TRUNCATE = false;

static const std::chrono::duration<int64_t> FIVE_SECONDS = std::chrono::seconds(5);

void setupLogger() {
   spdlog::cfg::load_env_levels();
   spdlog::flush_every(FIVE_SECONDS);

   auto file_logger = spdlog::daily_logger_mt(
      "file_logger", "logs/silo.log", AT_MIDNIGHT, AT_0_MINUTES, DONT_TRUNCATE, MAX_FILES_7
   );

   auto console_logger = spdlog::stdout_color_mt("console_logger");
   console_logger->flush_on(spdlog::level::trace);

   auto default_logger =
      file_logger->level() < console_logger->level() ? file_logger : console_logger;
   spdlog::set_default_logger(default_logger);

   spdlog::daily_logger_mt(
      silo::PERFORMANCE_LOGGER_NAME,
      "logs/performance.log",
      AT_MIDNIGHT,
      AT_0_MINUTES,
      DONT_TRUNCATE,
      MAX_FILES_7
   );
}
