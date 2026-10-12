#include "rhydb/schema/database_schema.h"

#include <map>
#include <memory>

#include <gtest/gtest.h>

#include "rhydb/storage/column/bool_column.h"
#include "rhydb/storage/column/column_metadata.h"
#include "rhydb/storage/column/int_column.h"

using rhydb::schema::ColumnIdentifier;
using rhydb::schema::ColumnType;
using rhydb::schema::TableSchema;
using rhydb::storage::column::BoolColumn;
using rhydb::storage::column::ColumnMetadata;

namespace {

TableSchema schemaWithBoolColumn() {
   const ColumnIdentifier column{.name = "flag", .type = ColumnType::BOOL};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata{
      {column, std::make_shared<ColumnMetadata>("flag")}
   };
   return TableSchema{column_metadata, std::nullopt};
}

}  // namespace

TEST(TableSchema, getColumnMetadataReturnsMetadataOfExistingColumn) {
   const auto schema = schemaWithBoolColumn();
   const auto metadata = schema.getColumnMetadata<BoolColumn>("flag");
   ASSERT_TRUE(metadata.has_value());
   EXPECT_EQ(metadata.value()->column_name, "flag");
}

TEST(TableSchema, getColumnMetadataReturnsNulloptForMissingColumn) {
   const auto schema = schemaWithBoolColumn();
   EXPECT_FALSE(schema.getColumnMetadata<BoolColumn>("does_not_exist").has_value());
}

TEST(TableSchema, getColumnMetadataReturnsNulloptForMismatchingType) {
   const auto schema = schemaWithBoolColumn();
   EXPECT_FALSE(schema.getColumnMetadata<rhydb::storage::column::Int32Column>("flag").has_value());
}
