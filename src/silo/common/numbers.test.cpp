#include <gtest/gtest.h>

#include "silo/common/numbers.h"

TEST(inc, signalsErrorWhenNeeded) {
   EXPECT_EQ(inc(0), 1);
   EXPECT_EQ(inc(4294967294), 4294967295);
   EXPECT_THROW(inc(4294967295), std::overflow_error);
}
