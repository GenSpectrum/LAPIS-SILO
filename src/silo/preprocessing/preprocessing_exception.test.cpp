#include "silo/preprocessing/preprocessing_exception.h"

#include <gtest/gtest.h>

void testFunction() {
   throw silo::PreprocessingException("SomeText");
}

TEST(PreprocessingException, assertThatItThrows) {
   EXPECT_THROW(testFunction(), silo::PreprocessingException);
}