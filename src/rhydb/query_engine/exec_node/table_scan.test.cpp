#include "rhydb/query_engine/exec_node/table_scan.h"

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/api.h>
#include <arrow/testing/gtest_util.h>
#include <fmt/format.h>
#include <gtest/gtest.h>

#include "rhydb/append/ndjson_line_reader.h"
#include "rhydb/append/table_inserter.h"
#include "rhydb/query_engine/operators/compute_filter.h"
#include "rhydb/query_engine/scalar_expressions/literal.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/column/string_column.h"
#include "rhydb/storage/table.h"

using rhydb::append::appendDataToTable;
using rhydb::append::NdjsonLineReader;
using rhydb::query_engine::exec_node::makeTableScan;
using rhydb::query_engine::operators::CompiledFilter;
using rhydb::query_engine::scalar_expressions::BoolLiteral;
using rhydb::schema::ColumnIdentifier;
using rhydb::schema::ColumnType;
using rhydb::schema::TableName;
using rhydb::schema::TableSchema;
using rhydb::storage::Table;
using rhydb::storage::column::ColumnMetadata;
using rhydb::storage::column::StringColumnMetadata;

namespace {

const ColumnIdentifier PRIMARY_KEY{.name = "id", .type = ColumnType::STRING};

// Rows are produced in batches of BATCH_SIZE_CUTOFF + 1
constexpr size_t BATCH_SIZE_CUTOFF = 1;
constexpr int ROWS = 10;
constexpr int BATCHES = 5;

std::shared_ptr<Table> makeTable() {
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata;
   column_metadata.emplace(PRIMARY_KEY, std::make_shared<StringColumnMetadata>(PRIMARY_KEY.name));
   auto table = std::make_shared<Table>(
      TableName::getDefault(),
      std::make_shared<TableSchema>(std::move(column_metadata), PRIMARY_KEY)
   );
   std::string ndjson;
   for (int row = 0; row < ROWS; ++row) {
      ndjson += fmt::format(R"({{"id":"row{}"}})", row) + "\n";
   }
   std::stringstream input{ndjson};
   NdjsonLineReader reader{input};
   appendDataToTable(table, reader);
   return table;
}

arrow::acero::ExecNode* addScan(arrow::acero::ExecPlan& plan, const std::shared_ptr<Table>& table) {
   return makeTableScan(
             &plan,
             {PRIMARY_KEY},
             CompiledFilter{table, BoolLiteral{true}},
             table,
             BATCH_SIZE_CUTOFF
   )
      .ValueOrDie();
}

// Pauses its input on the first batch and resumes it when asked to
class PausingSink final : public arrow::acero::ExecNode {
  public:
   PausingSink(arrow::acero::ExecPlan* plan, arrow::acero::ExecNode* input)
       : ExecNode(plan, {input}, {"input"}, /*output_schema=*/nullptr) {}

   [[nodiscard]] const char* kind_name() const override { return "PausingSink"; }

   arrow::Status InputReceived(arrow::acero::ExecNode* input, arrow::ExecBatch /*batch*/) override {
      if (received_batches++ == 0) {
         input->PauseProducing(this, 1);
      }
      return arrow::Status::OK();
   }

   arrow::Status InputFinished(arrow::acero::ExecNode* /*input*/, int total_batches) override {
      total = total_batches;
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override { return arrow::Status::OK(); }
   void PauseProducing(arrow::acero::ExecNode* /*output*/, int32_t /*counter*/) override {}
   void ResumeProducing(arrow::acero::ExecNode* /*output*/, int32_t /*counter*/) override {}

   void resume() { inputs_[0]->ResumeProducing(this, 2); }

   std::atomic<int> received_batches = 0;
   std::atomic<int> total = -1;

  protected:
   arrow::Status StopProducingImpl() override { return arrow::Status::OK(); }
};

// Long enough for the scan to produce everything if it ignored the pause
void giveTheScanTimeToRun() {
   std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

}  // namespace

TEST(TableScan, producesAllRowsMatchedByTheFilter) {
   auto table = makeTable();
   ASSERT_OK_AND_ASSIGN(auto plan, arrow::acero::ExecPlan::Make());
   std::shared_ptr<arrow::Table> output;
   ASSERT_OK(
      arrow::acero::MakeExecNode(
         "table_sink",
         plan.get(),
         {addScan(*plan, table)},
         arrow::acero::TableSinkNodeOptions{&output}
      )
         .status()
   );
   ASSERT_OK(plan->Validate());
   plan->StartProducing();
   ASSERT_OK(plan->finished().status());
   EXPECT_EQ(output->num_rows(), ROWS);
}

TEST(TableScan, stopsProducingWhilePausedAndContinuesOnResume) {
   auto table = makeTable();
   ASSERT_OK_AND_ASSIGN(auto plan, arrow::acero::ExecPlan::Make());
   auto* sink = plan->EmplaceNode<PausingSink>(plan.get(), addScan(*plan, table));
   ASSERT_OK(plan->Validate());
   plan->StartProducing();

   giveTheScanTimeToRun();
   // The batch after the one that paused may already be underway
   EXPECT_LE(sink->received_batches, 2);
   EXPECT_FALSE(plan->finished().is_finished());

   sink->resume();
   ASSERT_OK(plan->finished().status());
   EXPECT_EQ(sink->received_batches, BATCHES);
   EXPECT_EQ(sink->total, BATCHES);
}

TEST(TableScan, finishesWhenStoppedWhilePaused) {
   auto table = makeTable();
   ASSERT_OK_AND_ASSIGN(auto plan, arrow::acero::ExecPlan::Make());
   plan->EmplaceNode<PausingSink>(plan.get(), addScan(*plan, table));
   ASSERT_OK(plan->Validate());
   plan->StartProducing();

   giveTheScanTimeToRun();
   plan->StopProducing();
   EXPECT_TRUE(plan->finished().Wait(5));
}
