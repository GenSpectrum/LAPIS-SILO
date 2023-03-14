#include <iostream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class ClassToTest {
  public:
   virtual void functionToMock() {}
   virtual void callMockedFunction() { functionToMock(); }
};

class MockClassToTest : public ClassToTest {
  public:
   MOCK_METHOD(void, functionToMock, (), (override));
};

TEST(example_test, mock_example) {
   MockClassToTest mock;

   EXPECT_CALL(mock, functionToMock()).Times(testing::AtLeast(1));

   mock.callMockedFunction();
}

TEST(example_test, assert_example) {
   EXPECT_EQ(1, 1);
}