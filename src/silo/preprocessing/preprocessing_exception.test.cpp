#include "silo/preprocessing/preprocessing_exception.h"

#include <gtest/gtest.h>

void testFunction() {
   throw PreprocessingException("SomeText");
}

TEST(preprocessing_exception, assert_that_it_throws) {
   EXPECT_THROW(testFunction(), PreprocessingException);
}