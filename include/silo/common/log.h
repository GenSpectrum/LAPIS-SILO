#ifndef SILO_LOG_H
#define SILO_LOG_H

#include <string>

#include <spdlog/spdlog.h>

namespace silo {
static const std::string PERFORMANCE_LOGGER_NAME = "performance_logger";
}

#define LOG_PERFORMANCE(...) SPDLOG_LOGGER_INFO(spdlog::get(PERFORMANCE_LOGGER_NAME), __VA_ARGS__)

#endif  // SILO_LOG_H
