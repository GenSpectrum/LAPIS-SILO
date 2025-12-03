#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

#include <arrow/compute/api.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "silo/common/log.h"
#include "silo/common/panic.h"

namespace {

int changeCwdToTestFolder() {
   // Look for the test data directory (`testBaseData`) in the current directory and up to
   // <search_depth> directories above the current directory. If found, change the current working
   // directory to the directory containing the test data directory
   size_t search_depth = 3;
   std::filesystem::path candidate_directory = std::filesystem::current_path().string();
   for (size_t i = 0; i < search_depth; i++, candidate_directory = candidate_directory / "..") {
      if (std::filesystem::exists(candidate_directory / "testBaseData/exampleDataset")) {
         std::filesystem::current_path(candidate_directory);
         return 0;
      }
   }
   SPDLOG_ERROR(
      "Should be run in root of repository, got {} and could not find root by heuristics",
      std::filesystem::current_path().string()
   );
   return 1;
}

}  // namespace

int main(int argc, char* argv[]) {
   if (auto exit = changeCwdToTestFolder()) {
      return exit;
   }
   if (spdlog::get_level() > spdlog::level::debug) {
      spdlog::set_level(spdlog::level::trace);
   }
   spdlog::null_logger_mt(silo::PERFORMANCE_LOGGER_NAME);
   SILO_ASSERT(arrow::compute::Initialize().ok());
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}
