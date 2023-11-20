#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "silo/common/log.h"

int main(int argc, char* argv[]) {
   spdlog::set_level(spdlog::level::off);
   spdlog::null_logger_mt(silo::PERFORMANCE_LOGGER_NAME);
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}