#include <gtest/gtest.h>

#include "silo/common/numbers.h"

using silo::common::add1;

TEST(add1, signalsErrorWhenNeeded) {
   EXPECT_EQ(add1(0), 1);
   EXPECT_EQ(add1(4294967294), 4294967295);
   EXPECT_THROW(add1(4294967295), std::overflow_error);
}
