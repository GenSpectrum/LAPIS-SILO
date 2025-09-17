#include "silo/query_engine/exec_node/throttled_batch_reslicer.h"

#include <arrow/acero/exec_plan.h>
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <arrow/testing/gtest_util.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>

using namespace silo::query_engine::exec_node;
using namespace arrow;
using namespace arrow::acero;
using namespace arrow::compute;

// Mock BackpressureMonitor for testing
class MockBackpressureMonitor : public BackpressureMonitor {
  public:
   MOCK_METHOD(uint64_t, bytes_in_use, (), (override));
   MOCK_METHOD(bool, is_paused, (), (override));
};

class ThrottledBatchReslicerTest : public ::testing::Test {
  protected:
   void SetUp() override {
      mock_backpressure_monitor = std::make_unique<MockBackpressureMonitor>();

      // Default mock behavior
      ON_CALL(*mock_backpressure_monitor, bytes_in_use()).WillByDefault(::testing::Return(1000));
      ON_CALL(*mock_backpressure_monitor, is_paused()).WillByDefault(::testing::Return(false));
   }

   std::unique_ptr<MockBackpressureMonitor> mock_backpressure_monitor;

   // Helper function to create a simple ExecBatch with integer data
   ExecBatch CreateTestBatch(int64_t length, int32_t start_value = 0) {
      auto builder = std::make_shared<arrow::Int32Builder>();
      for (int32_t i = 0; i < length; ++i) {
         ARROW_EXPECT_OK(builder->Append(start_value + i));
      }
      std::shared_ptr<arrow::Array> array;
      ARROW_EXPECT_OK(builder->Finish(&array));

      return ExecBatch({array}, length);
   }

   // Helper to create async generator from vector of batches
   AsyncGenerator<std::optional<ExecBatch>> CreateGenerator(
      std::vector<std::optional<ExecBatch>> batches
   ) {
      auto batches_wrapped = std::make_shared<decltype(batches)>(batches);
      auto it = std::make_shared<decltype(batches_wrapped->begin())>(batches_wrapped->begin());
      auto end_it = batches_wrapped->end();

      return [batches_wrapped, it, end_it]() -> Future<std::optional<ExecBatch>> {
         if (*it == end_it) {
            return std::optional<ExecBatch>{std::nullopt};
         }
         auto result = **it;
         ++(*it);
         return result;
      };
   }
};

TEST_F(ThrottledBatchReslicerTest, ConstructorValidation) {
   auto generator = CreateGenerator({});

   // Valid construction should not throw
   EXPECT_NO_THROW({
      ThrottledBatchReslicer reslicer(
         generator, 100, std::chrono::milliseconds(10), mock_backpressure_monitor.get()
      );
   });

   // batch_size = 0 should trigger assertion
   EXPECT_THAT(
      [&]() {
         ThrottledBatchReslicer reslicer(
            generator,
            0,  // Invalid batch_size
            std::chrono::milliseconds(10),
            mock_backpressure_monitor.get()
         );
      },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("ASSERT failure"))
   );
}

TEST_F(ThrottledBatchReslicerTest, EmptyInput) {
   auto generator = CreateGenerator({std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator, 100, std::chrono::milliseconds(10), mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();

   ASSERT_TRUE(result.ok());
   EXPECT_FALSE(result.ValueOrDie().has_value());
}

TEST_F(ThrottledBatchReslicerTest, EmptyBatch) {
   auto empty_batch = CreateTestBatch(0);
   auto generator = CreateGenerator({empty_batch, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator, 100, std::chrono::milliseconds(10), mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();

   ASSERT_TRUE(result.ok());
   auto batch = result.ValueOrDie();
   ASSERT_TRUE(batch.has_value());
   EXPECT_EQ(batch->length, 0);
}

TEST_F(ThrottledBatchReslicerTest, BatchSmallerThanTargetSize) {
   auto small_batch = CreateTestBatch(50);  // Smaller than target size
   auto generator = CreateGenerator({small_batch, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator,
      100,  // Target size larger than input
      std::chrono::milliseconds(10),
      mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();

   ASSERT_TRUE(result.ok());
   auto batch = result.ValueOrDie();
   ASSERT_TRUE(batch.has_value());
   EXPECT_EQ(batch->length, 50);

   // Should return nullopt on next call
   future = reslicer();
   result = future.result();
   ASSERT_TRUE(result.ok());
   EXPECT_FALSE(result.ValueOrDie().has_value());
}

TEST_F(ThrottledBatchReslicerTest, BatchEqualToTargetSize) {
   auto exact_batch = CreateTestBatch(100);
   auto generator = CreateGenerator({exact_batch, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator,
      100,  // Exact target size
      std::chrono::milliseconds(10),
      mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();

   ASSERT_TRUE(result.ok());
   auto batch = result.ValueOrDie();
   ASSERT_TRUE(batch.has_value());
   EXPECT_EQ(batch->length, 100);
}

TEST_F(ThrottledBatchReslicerTest, BatchLargerThanTargetSize) {
   auto large_batch = CreateTestBatch(250);
   auto generator = CreateGenerator({large_batch, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator,
      100,                           // Target size smaller than input
      std::chrono::milliseconds(1),  // Very short delay for testing
      mock_backpressure_monitor.get()
   );

   // First call should return first slice
   auto future1 = reslicer();
   auto result1 = future1.result();
   ASSERT_TRUE(result1.ok());
   auto batch1 = result1.ValueOrDie();
   ASSERT_TRUE(batch1.has_value());
   EXPECT_EQ(batch1->length, 100);

   // Second call should return second slice
   auto future2 = reslicer();
   auto result2 = future2.result();
   ASSERT_TRUE(result2.ok());
   auto batch2 = result2.ValueOrDie();
   ASSERT_TRUE(batch2.has_value());
   EXPECT_EQ(batch2->length, 100);

   // Third call should return remaining slice
   auto future3 = reslicer();
   auto result3 = future3.result();
   ASSERT_TRUE(result3.ok());
   auto batch3 = result3.ValueOrDie();
   ASSERT_TRUE(batch3.has_value());
   EXPECT_EQ(batch3->length, 50);

   // Fourth call should return nullopt (end of input)
   auto future4 = reslicer();
   auto result4 = future4.result();
   ASSERT_TRUE(result4.ok());
   EXPECT_FALSE(result4.ValueOrDie().has_value());
}

TEST_F(ThrottledBatchReslicerTest, MultipleBatches) {
   auto batch1 = CreateTestBatch(150, 0);
   auto batch2 = CreateTestBatch(75, 150);
   auto generator = CreateGenerator({batch1, batch2, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator, 100, std::chrono::milliseconds(1), mock_backpressure_monitor.get()
   );

   std::vector<int64_t> batch_sizes;

   // Process all batches
   while (true) {
      auto future = reslicer();
      auto result = future.result();
      ASSERT_TRUE(result.ok());
      auto batch = result.ValueOrDie();

      if (!batch.has_value()) {
         break;
      }

      batch_sizes.push_back(batch->length);
   }

   // Should get: 100 (from batch1), 50 (remainder of batch1), 75 (batch2)
   EXPECT_THAT(batch_sizes, ::testing::ElementsAre(100, 50, 75));
}

TEST_F(ThrottledBatchReslicerTest, ThrottlingDelay) {
   auto large_batch = CreateTestBatch(200);
   auto generator = CreateGenerator({large_batch, std::nullopt});

   const auto delay = std::chrono::milliseconds(50);
   ThrottledBatchReslicer reslicer(generator, 100, delay, mock_backpressure_monitor.get());

   auto start_time = std::chrono::system_clock::now();

   // First call should not have delay (no previous batch)
   auto future1 = reslicer();
   auto result1 = future1.result();
   ASSERT_TRUE(result1.ok());
   EXPECT_TRUE(result1.ValueOrDie().has_value());

   auto first_batch_time = std::chrono::system_clock::now();

   // Second call should have delay
   auto future2 = reslicer();
   auto result2 = future2.result();
   ASSERT_TRUE(result2.ok());
   EXPECT_TRUE(result2.ValueOrDie().has_value());

   auto second_batch_time = std::chrono::system_clock::now();

   // Check that delay was applied (with some tolerance)
   auto actual_delay = second_batch_time - first_batch_time;
   EXPECT_GE(actual_delay, delay - std::chrono::milliseconds(5));
}

TEST_F(ThrottledBatchReslicerTest, BackpressureMonitorLogging) {
   auto batch = CreateTestBatch(50);
   auto generator = CreateGenerator({batch, std::nullopt});

   // Set up expectations for backpressure monitor calls
   EXPECT_CALL(*mock_backpressure_monitor, bytes_in_use()).Times(::testing::AtLeast(1));
   EXPECT_CALL(*mock_backpressure_monitor, is_paused()).Times(::testing::AtLeast(1));

   ThrottledBatchReslicer reslicer(
      generator, 100, std::chrono::milliseconds(1), mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();
   ASSERT_TRUE(result.ok());
}

TEST_F(ThrottledBatchReslicerTest, DataIntegrity) {
   // Create a batch with known data
   auto builder = std::make_shared<arrow::Int32Builder>();
   for (int32_t i = 0; i < 150; ++i) {
      ARROW_EXPECT_OK(builder->Append(i));
   }
   std::shared_ptr<arrow::Array> array;
   ARROW_EXPECT_OK(builder->Finish(&array));

   auto batch = ExecBatch({array}, 150);
   auto generator = CreateGenerator({batch, std::nullopt});

   ThrottledBatchReslicer reslicer(
      generator, 100, std::chrono::milliseconds(1), mock_backpressure_monitor.get()
   );

   // Get first slice (0-99)
   auto future1 = reslicer();
   auto result1 = future1.result();
   ASSERT_TRUE(result1.ok());
   auto batch1 = result1.ValueOrDie();
   ASSERT_TRUE(batch1.has_value());
   EXPECT_EQ(batch1->length, 100);

   // Verify data integrity of first slice
   auto int_array1 = std::static_pointer_cast<arrow::Int32Array>(batch1->values[0].make_array());
   for (int32_t i = 0; i < 100; ++i) {
      EXPECT_EQ(int_array1->Value(i), i);
   }

   // Get second slice (100-149)
   auto future2 = reslicer();
   auto result2 = future2.result();
   ASSERT_TRUE(result2.ok());
   auto batch2 = result2.ValueOrDie();
   ASSERT_TRUE(batch2.has_value());
   EXPECT_EQ(batch2->length, 50);

   // Verify data integrity of second slice
   auto int_array2 = std::static_pointer_cast<arrow::Int32Array>(batch2->values[0].make_array());
   for (int32_t i = 0; i < 50; ++i) {
      EXPECT_EQ(int_array2->Value(i), 100 + i);
   }
}

TEST_F(ThrottledBatchReslicerTest, ExceptionHandling) {
   // Create a generator that throws an exception
   auto throwing_generator = []() -> Future<std::optional<ExecBatch>> {
      throw std::runtime_error("Test exception");
   };

   ThrottledBatchReslicer reslicer(
      throwing_generator, 100, std::chrono::milliseconds(10), mock_backpressure_monitor.get()
   );

   auto future = reslicer();
   auto result = future.result();

   // Should return an error status, not throw
   EXPECT_FALSE(result.ok());
   EXPECT_THAT(result.status().message(), ::testing::HasSubstr("Test exception"));
}

// Performance test to ensure throttling works correctly under load
TEST_F(ThrottledBatchReslicerTest, PerformanceThrottling) {
   auto large_batch = CreateTestBatch(1000);
   auto generator = CreateGenerator({large_batch, std::nullopt});

   const auto delay = std::chrono::milliseconds(10);
   ThrottledBatchReslicer reslicer(generator, 100, delay, mock_backpressure_monitor.get());

   auto start_time = std::chrono::system_clock::now();
   int batch_count = 0;

   while (true) {
      auto future = reslicer();
      auto result = future.result();
      ASSERT_TRUE(result.ok());
      auto batch = result.ValueOrDie();

      if (!batch.has_value()) {
         break;
      }

      batch_count++;
   }

   auto total_time = std::chrono::system_clock::now() - start_time;

   // Should have processed 10 batches (1000 / 100)
   EXPECT_EQ(batch_count, 10);

   // With 9 delays of 10ms each (first batch has no delay), total should be at least 90ms
   EXPECT_GE(total_time, std::chrono::milliseconds(90));
}
