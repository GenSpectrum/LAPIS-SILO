#include "silo/preprocessing/preprocessing_exception.h"

#include <gtest/gtest.h>

inline void configTestFunction() {
   throw silo::PreprocessingException("SomeText");
}

TEST(PreprocessingException, assertThatItThrows) {
   EXPECT_THROW(configTestFunction(), silo::PreprocessingException);
}