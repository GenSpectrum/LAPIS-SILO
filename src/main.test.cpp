#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "silo/common/log.h"

int main(int argc, char* argv[]) {
   try {
      if (!std::filesystem::exists("testBaseData/exampleDataset")) {
         throw std::runtime_error("must be run in root of repository");
      }
   } catch (std::exception& e) {
      SPDLOG_ERROR(e.what());
      return 1;
   }
   if (spdlog::get_level() > spdlog::level::debug) {
      spdlog::set_level(spdlog::level::trace);
   }
   spdlog::null_logger_mt(silo::PERFORMANCE_LOGGER_NAME);
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}
