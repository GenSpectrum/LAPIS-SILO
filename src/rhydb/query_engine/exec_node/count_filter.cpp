#include "rhydb/query_engine/exec_node/count_filter.h"

#include <cstdint>
#include <memory>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/compute/exec.h>
#include <arrow/datum.h>
#include <arrow/result.h>

#include "rhydb/common/bitmap.h"
#include "rhydb/query_engine/exec_node/computed_source.h"
#include "rhydb/query_engine/operators/compute_filter.h"

namespace rhydb::query_engine::exec_node {

using operators::CompiledFilter;

namespace {

class CountFilter final : public ComputedSourceNode {
  public:
   CountFilter(
      arrow::acero::ExecPlan* plan,
      CompiledFilter filter,
      std::shared_ptr<arrow::Schema> output_schema
   )
       : ComputedSourceNode(plan, std::move(output_schema)),
         filter_(std::move(filter)) {}

   [[nodiscard]] const char* kind_name() const override { return "CountFilter"; }

  private:
   arrow::Result<arrow::ExecBatch> compute() override {
      const Bitmap filter_bitmap = filter_.evaluate();

      arrow::Int64Builder result_builder{};
      ARROW_RETURN_NOT_OK(result_builder.Append(static_cast<int64_t>(filter_bitmap.cardinality())));
      ARROW_ASSIGN_OR_RAISE(arrow::Datum result, result_builder.Finish());
      return arrow::ExecBatch::Make({std::move(result)});
   }

   CompiledFilter filter_;
};

}  // namespace

arrow::acero::ExecNode* addCountFilterNode(
   arrow::acero::ExecPlan& plan,
   CompiledFilter filter,
   std::shared_ptr<arrow::Schema> output_schema
) {
   return plan.EmplaceNode<CountFilter>(&plan, std::move(filter), std::move(output_schema));
}

}  // namespace rhydb::query_engine::exec_node
