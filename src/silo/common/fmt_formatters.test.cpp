#include "silo/common/fmt_formatters.h"

#include <gtest/gtest.h>

using silo::common::toIsoString;

TEST(X, toIsoString) {
   auto specific_time_with_ns =
      std::chrono::system_clock::time_point() +
      std::chrono::nanoseconds(1700000000123456789);  // 2023-11-14T12:26:40.123456789Z
   ASSERT_EQ(toIsoString(specific_time_with_ns), "2023-11-14T22:13:20.123456789Z");
}
