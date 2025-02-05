#include "silo/query_engine/actions/aggregated.h"

#include <arrow/api.h>
#include <arrow/array/builder_binary.h>
#include <arrow/array/builder_primitive.h>
#include <arrow/compute/api.h>
#include <arrow/compute/api_aggregate.h>
#include <arrow/compute/api_vector.h>
#include <arrow/io/api.h>
#include <arrow/result.h>
#include <arrow/status.h>
#include <arrow/table.h>
#include <gtest/gtest.h>

arrow::Result<std::shared_ptr<arrow::Table>> CreateTable() {
   // Define column data
   std::vector<std::string> key_values = {"a", "b", "c", "a", "b", "c", "a", "b", "c", "a"};
   std::vector<int32_t> nums1_values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   std::vector<int32_t> nums2_values = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

   // Convert std::vectors to Arrow arrays
   std::shared_ptr<arrow::Array> key_array;
   std::shared_ptr<arrow::Array> nums1_array;
   std::shared_ptr<arrow::Array> nums2_array;

   arrow::StringBuilder key_builder;
   arrow::Int32Builder nums1_builder;
   arrow::Int32Builder nums2_builder;

   ARROW_RETURN_NOT_OK(key_builder.AppendValues(key_values));
   ARROW_RETURN_NOT_OK(nums1_builder.AppendValues(nums1_values));
   ARROW_RETURN_NOT_OK(nums2_builder.AppendValues(nums2_values));

   ARROW_RETURN_NOT_OK(key_builder.Finish(&key_array));
   ARROW_RETURN_NOT_OK(nums1_builder.Finish(&nums1_array));
   ARROW_RETURN_NOT_OK(nums2_builder.Finish(&nums2_array));

   // Define schema
   auto schema = arrow::schema(
      {arrow::field("key", arrow::utf8()),
       arrow::field("nums1", arrow::int32()),
       arrow::field("nums2", arrow::int32())}
   );

   // Create Table
   auto table = arrow::Table::Make(schema, {key_array, nums1_array, nums2_array});

   // Print schema and row count
   std::cout << "Table schema: " << table->schema()->ToString() << std::endl;
   std::cout << "Row count: " << table->num_rows() << std::endl;

   return table;
}

arrow::Result<std::shared_ptr<arrow::Table>> AggregateArrowTable(
   const std::shared_ptr<arrow::Table>& table,
   const std::vector<std::string>& group_by_columns
) {
   if (!table) {
      return arrow::Status::Invalid("Input table is null");
   }

   // Mapping column names to indices
   std::unordered_map<std::string, int> column_map;
   for (int i = 0; i < table->num_columns(); ++i) {
      column_map[table->schema()->field(i)->name()] = i;
   }

   // Convert column names to indices
   std::vector<std::shared_ptr<arrow::Field>> group_by_fields;
   std::vector<arrow::Datum> group_by_arrays;
   for (const auto& col_name : group_by_columns) {
      auto it = column_map.find(col_name);
      if (it == column_map.end()) {
         return arrow::Status::Invalid("Column not found: " + col_name);
      }
      group_by_fields.push_back(table->schema()->field(it->second));
      group_by_arrays.push_back(table->column(it->second));
   }

   // Identify numeric columns for aggregation
   std::vector<arrow::compute::Aggregate> aggregates;
   std::vector<arrow::Datum> aggregate_arrays;
   std::vector<std::shared_ptr<arrow::Field>> aggregate_fields;
   for (int i = 0; i < table->num_columns(); ++i) {
      if (column_map.find(table->schema()->field(i)->name()) == column_map.end()) {
         aggregates.push_back(
            arrow::compute::Aggregate{"sum", nullptr, table->schema()->field(i)->name() + "_sum"}
         );
         aggregate_arrays.push_back(table->column(i));
         aggregate_fields.push_back(arrow::field(
            table->schema()->field(i)->name() + "_sum", table->schema()->field(i)->type()
         ));
      }
   }

   /* ARROW_ASSIGN_OR_RAISE(
      auto result,
      arrow::compute::HashAggregateFunction(
         {group_by_datum},
         {aggregate_datum},
         aggregates,
         &ctx));

   // Convert result to Table
   return result.table(); */
   SILO_TODO();
}

arrow::Status test(){
   auto table = CreateTable().ValueUnsafe();
   std::cout << table->ToString() << std::endl;
   arrow::Datum result;
   std::shared_ptr<arrow::Scalar> increment = arrow::MakeScalar(3);

   arrow::compute::CountOptions scalar_aggregate_options;

   arrow::Datum min_max;

   ARROW_ASSIGN_OR_RAISE(
      min_max,
      arrow::compute::Count({table->column(0)}, scalar_aggregate_options)
   );

   std::cout << "Res:" << std::endl;
   std::cout << min_max.ToString() << std::endl;

   return arrow::Status::OK();
}

TEST(CreatesTable, test) {
   std::cout << test().ToString() << std::endl;
}
