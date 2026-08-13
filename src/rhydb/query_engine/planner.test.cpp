#include "rhydb/query_engine/planner.h"

#include <map>
#include <memory>
#include <stdexcept>

#include <arrow/acero/exec_plan.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "rhydb/config/runtime_config.h"
#include "rhydb/query_engine/operators/query_node.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

using rhydb::query_engine::Planner;
namespace operators = rhydb::query_engine::operators;

namespace {

class ErrorQueryNode final : public operators::QueryNode {
  public:
   [[nodiscard]] std::vector<rhydb::schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& /*plan*/,
      const std::map<rhydb::schema::TableName, std::shared_ptr<rhydb::storage::Table>>& /*tables*/,
      const rhydb::config::QueryOptions& /*query_options*/
   ) const override {
      return arrow::Status::ExecutionError("induced test error");
   }

   [[nodiscard]] operators::NodeKind kind() const override {
      return operators::NodeKind::COUNT_FILTER;
   }

   [[nodiscard]] nlohmann::json toJson() const override { return {{"type", "ErrorQueryNode"}}; }
};

TEST(PlannerPlanQuery, arrowErrorThrows) {
   rhydb::config::QueryOptions options{.materialization_cutoff = 1024};
   auto node = std::make_unique<ErrorQueryNode>();
   EXPECT_THAT(
      [&]() { (void)Planner::planQuery(std::move(node), {}, options, "test"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Error when planning query execution"))
   );
}

}  // namespace
