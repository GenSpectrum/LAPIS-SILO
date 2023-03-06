#include "gtest/gtest.h"
#include <silo/preprocessing/preprocessing_exception.h>

void testFunction() {
   throw PreprocessingException("SomeText");
}

TEST(preprocessing_exception, assert_that_it_throws) {
   EXPECT_THROW(testFunction(), PreprocessingException);
}