#include "rhydb/query_engine/exec_node/background_first_pull.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <arrow/table.h>
#include <gtest/gtest.h>

using rhydb::query_engine::exec_node::pullFirstBatchInBackground;
using rhydb::query_engine::exec_node::SourceGenerator;

namespace {

const auto SCHEMA = arrow::schema({arrow::field("value", arrow::int32())});

arrow::ExecBatch makeBatch(int32_t value) {
   arrow::Int32Builder builder;
   EXPECT_TRUE(builder.Append(value).ok());
   return arrow::ExecBatch::Make({builder.Finish().ValueOrDie()}).ValueOrDie();
}

/// Lets each participant wait until all of them have arrived, giving up after a timeout
class Rendezvous {
   std::mutex mutex;
   std::condition_variable all_arrived;
   int missing;

  public:
   explicit Rendezvous(int participants)
       : missing(participants) {}

   bool arriveAndWait() {
      std::unique_lock lock(mutex);
      if (--missing == 0) {
         all_arrived.notify_all();
         return true;
      }
      return all_arrived.wait_for(lock, std::chrono::seconds(10), [this] { return missing == 0; });
   }
};

/// Produces a single batch, but only once `rendezvous` was reached by all sources
SourceGenerator makeRendezvousGenerator(
   Rendezvous& rendezvous,
   std::atomic<bool>& all_met,
   int32_t value
) {
   return [&rendezvous,
           &all_met,
           value,
           produced = false] mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (produced) {
         return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(std::nullopt);
      }
      produced = true;
      if (!rendezvous.arriveAndWait()) {
         all_met = false;
      }
      return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(makeBatch(value));
   };
}

arrow::acero::Declaration makeSource(SourceGenerator generator) {
   return {"source", arrow::acero::SourceNodeOptions{SCHEMA, std::move(generator)}};
}

}  // namespace

TEST(PullFirstBatchInBackground, letsTheInputsOfAUnionProduceConcurrently) {
   if (arrow::internal::GetCpuThreadPool()->GetCapacity() < 2) {
      GTEST_SKIP() << "needs at least two CPU threads";
   }
   Rendezvous rendezvous{2};
   std::atomic<bool> all_met = true;

   const arrow::acero::Declaration plan{
      "union",
      {
         makeSource(pullFirstBatchInBackground(makeRendezvousGenerator(rendezvous, all_met, 1))),
         makeSource(pullFirstBatchInBackground(makeRendezvousGenerator(rendezvous, all_met, 2))),
      },
      arrow::acero::ExecNodeOptions{},
   };
   auto result = arrow::acero::DeclarationToTable(plan);

   ASSERT_TRUE(result.ok()) << result.status().ToString();
   EXPECT_EQ(result.ValueUnsafe()->num_rows(), 2);
   EXPECT_TRUE(all_met) << "the sources did not produce their first batch concurrently";
}

TEST(PullFirstBatchInBackground, passesThroughAllBatches) {
   SourceGenerator generator = [next =
                                   0] mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (next == 3) {
         return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(std::nullopt);
      }
      return arrow::Future<std::optional<arrow::ExecBatch>>::MakeFinished(makeBatch(next++));
   };

   auto result = arrow::acero::DeclarationToTable(
      makeSource(pullFirstBatchInBackground(std::move(generator)))
   );

   ASSERT_TRUE(result.ok()) << result.status().ToString();
   EXPECT_EQ(result.ValueUnsafe()->num_rows(), 3);
}

TEST(PullFirstBatchInBackground, turnsAnExceptionIntoAnErrorStatus) {
   const SourceGenerator generator = [] -> arrow::Future<std::optional<arrow::ExecBatch>> {
      throw std::runtime_error("some error");
   };

   auto result =
      arrow::acero::DeclarationToTable(makeSource(pullFirstBatchInBackground(generator)));

   ASSERT_FALSE(result.ok());
   EXPECT_TRUE(result.status().IsExecutionError());
   EXPECT_NE(result.status().message().find("some error"), std::string::npos);
}
