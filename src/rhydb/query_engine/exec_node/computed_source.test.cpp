#include "rhydb/query_engine/exec_node/computed_source.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/api.h>
#include <arrow/testing/gtest_util.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using rhydb::query_engine::exec_node::ComputedSourceNode;

namespace {

class TestSource final : public ComputedSourceNode {
  public:
   TestSource(
      arrow::acero::ExecPlan* plan,
      std::function<arrow::Result<arrow::ExecBatch>()> compute
   )
       : ComputedSourceNode(plan, arrow::schema({arrow::field("value", arrow::int64())})),
         compute_(std::move(compute)) {}

   [[nodiscard]] const char* kind_name() const override { return "TestSource"; }

  private:
   arrow::Result<arrow::ExecBatch> compute() override { return compute_(); }

   std::function<arrow::Result<arrow::ExecBatch>()> compute_;
};

arrow::ExecBatch makeBatch(int64_t length) {
   arrow::Int64Builder builder;
   for (int64_t value = 0; value < length; ++value) {
      ARROW_EXPECT_OK(builder.Append(value));
   }
   return arrow::ExecBatch({builder.Finish().ValueOrDie()}, length);
}

arrow::Result<std::shared_ptr<arrow::Table>> run(
   const std::vector<std::function<arrow::Result<arrow::ExecBatch>()>>& computations
) {
   ARROW_ASSIGN_OR_RAISE(auto plan, arrow::acero::ExecPlan::Make());
   std::vector<arrow::acero::ExecNode*> sources;
   sources.reserve(computations.size());
   for (const auto& compute : computations) {
      sources.push_back(plan->EmplaceNode<TestSource>(plan.get(), compute));
   }
   ARROW_ASSIGN_OR_RAISE(
      auto* node,
      arrow::acero::MakeExecNode("union", plan.get(), sources, arrow::acero::ExecNodeOptions{})
   );
   std::shared_ptr<arrow::Table> output;
   ARROW_RETURN_NOT_OK(
      arrow::acero::MakeExecNode(
         "table_sink", plan.get(), {node}, arrow::acero::TableSinkNodeOptions{&output}
      )
         .status()
   );
   ARROW_RETURN_NOT_OK(plan->Validate());
   plan->StartProducing();
   ARROW_RETURN_NOT_OK(plan->finished().status());
   return output;
}

}  // namespace

TEST(ComputedSourceNode, emitsTheComputedRowsInSlices) {
   const int64_t rows = (2 * arrow::acero::ExecPlan::kMaxBatchSize) + 1;
   ASSERT_OK_AND_ASSIGN(auto output, run({[rows] { return makeBatch(rows); }}));
   EXPECT_EQ(output->num_rows(), rows);
}

TEST(ComputedSourceNode, emitsNothingForAnEmptyResult) {
   ASSERT_OK_AND_ASSIGN(auto output, run({[] { return makeBatch(0); }}));
   EXPECT_EQ(output->num_rows(), 0);
}

TEST(ComputedSourceNode, failsThePlanWhenTheComputationFails) {
   const auto result =
      run({[] -> arrow::Result<arrow::ExecBatch> { return arrow::Status::Invalid("broken"); }});
   EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, testing::HasSubstr("broken"), result);
}

TEST(ComputedSourceNode, failsThePlanWhenTheComputationThrows) {
   const auto result =
      run({[] -> arrow::Result<arrow::ExecBatch> { throw std::runtime_error("thrown"); }});
   EXPECT_RAISES_WITH_MESSAGE_THAT(ExecutionError, testing::HasSubstr("thrown"), result);
}

TEST(ComputedSourceNode, computesIndependentSourcesConcurrently) {
   std::mutex mutex;
   std::condition_variable all_started;
   int started = 0;
   const auto wait_for_each_other = [&] -> arrow::Result<arrow::ExecBatch> {
      std::unique_lock lock(mutex);
      ++started;
      all_started.notify_all();
      if (!all_started.wait_for(lock, std::chrono::seconds(10), [&] { return started == 2; })) {
         return arrow::Status::Invalid("the sources were computed one after another");
      }
      return makeBatch(1);
   };
   ASSERT_OK_AND_ASSIGN(auto output, run({wait_for_each_other, wait_for_each_other}));
   EXPECT_EQ(output->num_rows(), 2);
}
