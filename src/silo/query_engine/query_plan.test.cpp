#include "silo/query_engine/query_plan.h"

#include <arrow/table.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "arrow/acero/options.h"
#include "arrow/builder.h"

using arrow::acero::ExecNode;
using arrow::acero::ExecPlan;
using silo::query_engine::QueryPlan;

arrow::Result<std::shared_ptr<arrow::Table>> setupTestTable() {
   std::shared_ptr<arrow::Schema> schema = arrow::schema({arrow::field("id", arrow::int32())});

   arrow::Int32Builder id_builder;
   for (size_t id = 1; id <= 100000; ++id) {
      ARROW_RETURN_NOT_OK(id_builder.Append(id++));
   }
   ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::Array> id_array, id_builder.Finish());
   return arrow::Table::Make(schema, {id_array});
}

TEST(QueryPlan, timesOutWhenAnInvalidPlanDoesNotFinish) {
   EXPECT_THAT(
      ([]() {
         auto arrow_plan = arrow::acero::ExecPlan::Make().ValueOrDie();
         auto table = setupTestTable().ValueOrDie();
         auto node =
            arrow::acero::MakeExecNode(
               "table_source", arrow_plan.get(), {}, arrow::acero::TableSourceNodeOptions{table}
            )
               .ValueOrDie();

         auto count_options = std::make_shared<arrow::compute::CountOptions>(
            arrow::compute::CountOptions::CountMode::ALL
         );
         arrow::compute::Aggregate aggregate{
            "hash_count_all", count_options, std::vector<arrow::FieldRef>{}, "count"
         };
         arrow::acero::AggregateNodeOptions aggregate_node_options(
            {aggregate}, {arrow::FieldRef{"id"}}
         );
         node = arrow::acero::MakeExecNode(
                   "aggregate", arrow_plan.get(), {node}, aggregate_node_options
         )
                   .ValueOrDie();

         auto under_test = QueryPlan::makeQueryPlan(arrow_plan, node, "some_id").ValueOrDie();

         std::stringstream dummy_output{};
         // Set time-out to zero, so it immediately cancels execution (only works with pipeline
         // breakers like aggregate, because otherwise the StartProducing call might already do all
         // the work)
         under_test.executeAndWrite(&dummy_output, 0);
      }),
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr(
         "Internal server error. Please notify developers. SILO likely constructed an invalid "
         "arrow plan and more user-input validation needs to be added: Request timed out, no batch"
         " within 0 seconds."
      ))
   );
}
