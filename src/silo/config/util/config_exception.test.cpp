#include "silo/config/util/config_exception.h"

#include <gtest/gtest.h>

inline void testFunction() {
   throw silo::config::ConfigException("SomeText");
}

TEST(ConfigException, assertThatItThrows) {
   EXPECT_THROW(testFunction(), silo::config::ConfigException);
}