#include "rhydb/query_engine/scalar_expressions/literal.h"

#include <arrow/compute/api.h>
#include <arrow/scalar.h>
#include <arrow/type.h>
#include <gtest/gtest.h>

#include "rhydb/query_engine/exec_node/arrow_util.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

namespace {

using rhydb::schema::ColumnType;

TEST(NullLiteral, hasTheTypeItIsGiven) {
   for (const auto type : {ColumnType::INT32, ColumnType::FLOAT, ColumnType::STRING}) {
      const NullLiteral literal{type};
      EXPECT_EQ(literal.type(), type);
      EXPECT_EQ(literal.clone()->type(), type);
      EXPECT_EQ(literal.toString(), "null");
      EXPECT_TRUE(literal.freeIUs().empty());
   }
}

TEST(NullLiteral, evaluatesToANullOfItsType) {
   for (const auto type : {ColumnType::INT32, ColumnType::FLOAT, ColumnType::STRING}) {
      const auto expression = NullLiteral{type}.toArrowExpression().ValueOrDie();
      const auto bound = expression.Bind(arrow::Schema{{}}).ValueOrDie();
      const auto value =
         arrow::compute::ExecuteScalarExpression(bound, arrow::compute::ExecBatch{{}, 1})
            .ValueOrDie();
      ASSERT_TRUE(value.is_scalar());
      EXPECT_TRUE(
         value.scalar()->Equals(*arrow::MakeNullScalar(exec_node::columnTypeToArrowType(type)))
      );
   }
}

}  // namespace

}  // namespace rhydb::query_engine::scalar_expressions
