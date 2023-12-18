#include "silo/preprocessing/preprocessing_exception.h"

#include <gtest/gtest.h>

inline void configTestFunction() {
   throw silo::preprocessing::PreprocessingException("SomeText");
}

TEST(PreprocessingException, assertThatItThrows) {
   EXPECT_THROW(configTestFunction(), silo::preprocessing::PreprocessingException);
}