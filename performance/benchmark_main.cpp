// Entry point for the merged performance benchmark binary (rhydb_benchmark).

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <arrow/compute/api.h>

#include "sequence_generator.h"

#include "rhydb/common/panic.h"

int main(int argc, char** argv) {
   changeCwdToTestFolder();
   RHYDB_ASSERT(arrow::compute::Initialize().ok());
   ::testing::InitGoogleMock(&argc, argv);
   return RUN_ALL_TESTS();
}
