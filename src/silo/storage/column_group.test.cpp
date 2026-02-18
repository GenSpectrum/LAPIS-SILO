#include "silo/storage/column_group.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <simdjson.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/bool_column.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/string_column.h"

using silo::Nucleotide;
using silo::schema::ColumnIdentifier;
using silo::storage::ColumnPartitionGroup;
using silo::storage::column::BoolColumnPartition;
using silo::storage::column::Column;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::DateColumnPartition;
using silo::storage::column::FloatColumnPartition;
using silo::storage::column::IntColumnPartition;
using silo::storage::column::SequenceColumnMetadata;
using silo::storage::column::SequenceColumnPartition;
using silo::storage::column::StringColumnMetadata;
using silo::storage::column::StringColumnPartition;

namespace {

template <Column ColumnType>
std::expected<void, std::string> setupColumnAndInsertJson(
   const std::string& column_name,
   const std::string& json_string
) {
   std::unique_ptr<typename ColumnType::Metadata> meta;
   if constexpr (std::is_same_v<ColumnType, SequenceColumnPartition<Nucleotide>>) {
      meta = std::make_unique<SequenceColumnMetadata<Nucleotide>>(
         column_name, std::vector<Nucleotide::Symbol>{Nucleotide::Symbol::A}
      );
   } else {
      meta = std::make_unique<typename ColumnType::Metadata>(column_name);
   }
   ColumnPartitionGroup partition_group;
   partition_group.getColumns<ColumnType>().emplace(column_name, ColumnType{meta.get()});

   simdjson::ondemand::parser parser;
   const simdjson::padded_string json(json_string);
   auto doc = parser.iterate(json).value_unsafe();
   simdjson::ondemand::value val = doc[column_name].value_unsafe();

   return partition_group.addJsonValueToColumn(
      ColumnIdentifier{column_name, ColumnType::TYPE}, val
   );
}

}  // namespace

TEST(ColumnPartitionGroup, givenIntegerValueForBoolColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<BoolColumnPartition>("bool_col", R"({"bool_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'bool_col': error getting value as boolean: 42."
      )
   );
}

TEST(ColumnPartitionGroup, givenStringValueForIntColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<IntColumnPartition>("int_col", R"({"int_col": "hello"})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'int_col': error getting value as int32: \"hello\"."
      )
   );
}

TEST(ColumnPartitionGroup, givenStringValueForFloatColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<FloatColumnPartition>("float_col", R"({"float_col": "hello"})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'float_col': error getting value as double: \"hello\"."
      )
   );
}

TEST(ColumnPartitionGroup, givenIntegerValueForStringColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<StringColumnPartition>("string_col", R"({"string_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'string_col': error getting value as string: 42."
      )
   );
}

TEST(ColumnPartitionGroup, givenIntegerValueForDateColumn_returnsColumnInsertError) {
   const auto result =
      setupColumnAndInsertJson<DateColumnPartition>("date_col", R"({"date_col": 42})");

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'date_col': error getting value as string: 42."
      )
   );
}

TEST(ColumnPartitionGroup, givenObjectMissingSequenceField_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<SequenceColumnPartition<Nucleotide>>(
      "nuc_col", R"({"nuc_col": {}})"
   );

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'nuc_col': error getting field 'sequence' in object:"
      )
   );
}

TEST(ColumnPartitionGroup, givenObjectMissingInsertionsField_returnsColumnInsertError) {
   const auto result = setupColumnAndInsertJson<SequenceColumnPartition<Nucleotide>>(
      "nuc_col", R"({"nuc_col": {"sequence": "A"}})"
   );

   ASSERT_FALSE(result.has_value());
   EXPECT_THAT(
      result.error(),
      testing::HasSubstr(
         "error inserting into column 'nuc_col': error getting field 'insertions' in object:"
      )
   );
}
