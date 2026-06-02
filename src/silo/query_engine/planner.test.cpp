#include "silo/query_engine/planner.h"

#include <map>
#include <memory>
#include <stdexcept>

#include <arrow/result.h>
#include <arrow/status.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "silo/config/runtime_config.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

using silo::query_engine::Planner;
namespace operators = silo::query_engine::operators;

namespace {

class ErrorQueryNode final : public operators::QueryNode {
  public:
   [[nodiscard]] std::vector<silo::schema::ColumnIdentifier> getOutputSchema() const override {
      return {};
   }

   [[nodiscard]] arrow::Result<operators::PartialArrowPlan> toQueryPlan(
      const std::map<silo::schema::TableName, std::shared_ptr<silo::storage::Table>>& /*tables*/,
      const silo::config::QueryOptions& /*query_options*/
   ) const override {
      return arrow::Status::ExecutionError("induced test error");
   }

   [[nodiscard]] operators::NodeKind kind() const override {
      return operators::NodeKind::COUNT_FILTER;
   }

   [[nodiscard]] nlohmann::json toJson() const override { return {{"type", "ErrorQueryNode"}}; }
};

TEST(PlannerPlanQuery, arrowErrorThrows) {
   silo::config::QueryOptions options{.materialization_cutoff = 1024};
   auto node = std::make_unique<ErrorQueryNode>();
   EXPECT_THAT(
      [&]() { (void)Planner::planQuery(std::move(node), {}, options, "test"); },
      ThrowsMessage<std::runtime_error>(::testing::HasSubstr("Error when planning query execution"))
   );
}

}  // namespace
