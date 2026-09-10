// Entry point for the merged performance benchmark binary (rhydb_benchmark). Every benchmark is a
// GTest TEST in its own translation unit; this file provides the single main() that they all share.
//
// The per-benchmark run() functions used to do these startup steps themselves; now that they are
// one executable, the startup happens once here before the tests run:
//   - changeCwdToTestFolder() so benchmarks find testBaseData/, localTestData/ and
//     performance/mutations.csv via their relative paths, wherever the binary was invoked from.
//   - arrow::compute::Initialize() so Arrow's compute kernels (e.g. utf8_slice_codeunits used by
//     the `at` scalar function) are registered before any benchmark plans a query.

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
