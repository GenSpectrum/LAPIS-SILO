#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
   spdlog::set_level(spdlog::level::off);

   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}