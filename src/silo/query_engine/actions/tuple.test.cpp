#include "silo/query_engine/actions/tuple.h"

#include <gtest/gtest.h>

#include "silo/config/database_config.h"
#include "silo/query_engine/actions/action.h"

// NOLINTBEGIN(bugprone-unchecked-optional-access)

using silo::schema::ColumnIdentifier;
using silo::schema::ColumnType;
using silo::schema::TableSchema;
using silo::storage::ColumnPartitionGroup;
using silo::storage::column::ColumnMetadata;
using silo::storage::column::IndexedStringColumnMetadata;
using silo::storage::column::StringColumnMetadata;

namespace {
std::pair<TableSchema, ColumnPartitionGroup> createSinglePartitionColumns() {
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata;
   ColumnPartitionGroup columns;
   const std::string primary_key_column_name = "primary_key_column";
   ColumnIdentifier primary_key_column_identifier{primary_key_column_name, ColumnType::STRING};
   {
      columns.metadata.push_back(primary_key_column_identifier);
      auto metadata = std::make_shared<StringColumnMetadata>(primary_key_column_name);
      columns.string_columns.emplace(primary_key_column_name, metadata.get());
      column_metadata.emplace(primary_key_column_identifier, metadata);

      columns.string_columns.at(primary_key_column_name).insert("k1");
      columns.string_columns.at(primary_key_column_name).insert("k2");
      columns.string_columns.at(primary_key_column_name).insert("k3");
   }
   {
      const std::string string_column_name = "dummy_string_column";
      ColumnIdentifier column_identifier{string_column_name, ColumnType::STRING};
      columns.metadata.push_back(column_identifier);
      auto metadata = std::make_shared<StringColumnMetadata>(string_column_name);
      columns.string_columns.emplace(string_column_name, metadata.get());
      column_metadata.emplace(column_identifier, metadata);

      columns.string_columns.at(string_column_name).insert("ABCD");
      columns.string_columns.at(string_column_name)
         .insert(
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string "
         );
      columns.string_columns.at(string_column_name).insert("ABCD");
   }
   {
      const std::string indexed_string_column_name = "dummy_indexed_string_column";
      ColumnIdentifier column_identifier{indexed_string_column_name, ColumnType::INDEXED_STRING};
      columns.metadata.push_back(column_identifier);
      auto metadata = std::make_shared<IndexedStringColumnMetadata>(indexed_string_column_name);
      columns.indexed_string_columns.emplace(indexed_string_column_name, metadata.get());
      column_metadata.emplace(column_identifier, metadata);

      columns.indexed_string_columns.at(indexed_string_column_name).insert("ABCD");
      columns.indexed_string_columns.at(indexed_string_column_name)
         .insert(
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string some very long string some very long string some very long "
            "string "
            "some very long string "
         );
      columns.indexed_string_columns.at(indexed_string_column_name).insert("ABCD");
   }
   {
      const std::string int_column_name = "dummy_int_column";
      ColumnIdentifier column_identifier{int_column_name, ColumnType::INT};
      columns.metadata.push_back(column_identifier);
      auto metadata = std::make_shared<ColumnMetadata>(int_column_name);
      columns.int_columns.emplace(int_column_name, metadata.get());
      column_metadata.emplace(column_identifier, metadata);
      columns.int_columns.at(int_column_name).insert(42);
      columns.int_columns.at(int_column_name).insert(-12389172);
      columns.int_columns.at(int_column_name).insert(42);
   }
   {
      const std::string float_column_name = "dummy_float_column";
      ColumnIdentifier column_identifier{float_column_name, ColumnType::FLOAT};
      columns.metadata.push_back(column_identifier);
      auto metadata = std::make_shared<ColumnMetadata>(float_column_name);
      columns.float_columns.emplace(float_column_name, metadata.get());
      column_metadata.emplace(column_identifier, metadata);
      columns.float_columns.at(float_column_name).insert(42.1);
      columns.float_columns.at(float_column_name).insert(-12389172.24222);
      columns.float_columns.at(float_column_name).insert(42.1);
   }
   {
      const std::string date_column_name = "dummy_date_column";
      ColumnIdentifier column_identifier{date_column_name, ColumnType::DATE};
      columns.metadata.push_back(column_identifier);
      auto metadata = std::make_shared<StringColumnMetadata>(date_column_name);
      columns.date_columns.emplace(date_column_name, metadata.get());
      column_metadata.emplace(column_identifier, metadata);
      columns.date_columns.at(date_column_name).insert("2023-01-01");
      columns.date_columns.at(date_column_name).insert("2023-01-01");
      columns.date_columns.at(date_column_name).insert("2023-01-01");
   }

   return {TableSchema{column_metadata, primary_key_column_identifier}, columns};
}
}  // namespace

using silo::query_engine::actions::Tuple;
using silo::query_engine::actions::TupleFactory;

TEST(Tuple, getsCreatedAndReturnsItsFieldsSuccessfully1) {
   auto columns = createSinglePartitionColumns();
   TupleFactory factory(columns.second, columns.second.metadata);
   const Tuple under_test = factory.allocateOne(0);

   auto fields = under_test.getFields();
   ASSERT_TRUE(fields.contains("dummy_date_column"));
   ASSERT_TRUE(fields.at("dummy_date_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_date_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_date_column").value()), "2023-01-01");

   ASSERT_TRUE(fields.contains("dummy_float_column"));
   ASSERT_TRUE(fields.at("dummy_float_column").has_value());
   ASSERT_TRUE(holds_alternative<double>(fields.at("dummy_float_column").value()));
   ASSERT_EQ(get<double>(fields.at("dummy_float_column").value()), 42.1);

   ASSERT_TRUE(fields.contains("dummy_int_column"));
   ASSERT_TRUE(fields.at("dummy_int_column").has_value());
   ASSERT_TRUE(holds_alternative<int32_t>(fields.at("dummy_int_column").value()));
   ASSERT_EQ(get<int32_t>(fields.at("dummy_int_column").value()), 42);

   ASSERT_TRUE(fields.contains("dummy_indexed_string_column"));
   ASSERT_TRUE(fields.at("dummy_indexed_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_indexed_string_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_indexed_string_column").value()), "ABCD");

   ASSERT_TRUE(fields.contains("dummy_string_column"));
   ASSERT_TRUE(fields.at("dummy_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_string_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_string_column").value()), "ABCD");
}

TEST(Tuple, getsCreatedAndReturnsItsFieldsSuccessfully2) {
   auto columns = createSinglePartitionColumns();
   TupleFactory factory(columns.second, columns.second.metadata);
   const Tuple under_test = factory.allocateOne(1);

   auto fields = under_test.getFields();
   ASSERT_TRUE(fields.contains("dummy_date_column"));
   ASSERT_TRUE(fields.at("dummy_date_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_date_column").value()));
   ASSERT_EQ(get<std::string>(fields.at("dummy_date_column").value()), "2023-01-01");

   ASSERT_TRUE(fields.contains("dummy_float_column"));
   ASSERT_TRUE(fields.at("dummy_float_column").has_value());
   ASSERT_TRUE(holds_alternative<double>(fields.at("dummy_float_column").value()));
   ASSERT_EQ(get<double>(fields.at("dummy_float_column").value()), -12389172.24222);

   ASSERT_TRUE(fields.contains("dummy_int_column"));
   ASSERT_TRUE(fields.at("dummy_int_column").has_value());
   ASSERT_TRUE(holds_alternative<int32_t>(fields.at("dummy_int_column").value()));
   ASSERT_EQ(get<int32_t>(fields.at("dummy_int_column").value()), -12389172);

   ASSERT_TRUE(fields.contains("dummy_indexed_string_column"));
   ASSERT_TRUE(fields.at("dummy_indexed_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_indexed_string_column").value()));
   ASSERT_EQ(
      get<std::string>(fields.at("dummy_indexed_string_column").value()),
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string "
   );

   ASSERT_TRUE(fields.contains("dummy_string_column"));
   ASSERT_TRUE(fields.at("dummy_string_column").has_value());
   ASSERT_TRUE(holds_alternative<std::string>(fields.at("dummy_string_column").value()));
   ASSERT_EQ(
      get<std::string>(fields.at("dummy_string_column").value()),
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string some very long string some very long string some very long "
      "string "
      "some very long string "
   );
}

TEST(TupleFactory, allocatesOneAllocatesManyEqual) {
   auto columns = createSinglePartitionColumns();
   TupleFactory factory(columns.second, columns.second.metadata);
   const Tuple under_test1 = factory.allocateOne(0);
   auto under_test_vector = factory.allocateMany(1);
   ASSERT_EQ(under_test_vector.size(), 1);
   factory.overwrite(under_test_vector.front(), 0);
   ASSERT_EQ(under_test1, under_test_vector.front());
}

TEST(Tuple, equalityOperatorEquatesCorrectly) {
   auto columns = createSinglePartitionColumns();
   auto all_columns = columns.second.metadata;
   std::erase(all_columns, columns.first.primary_key);
   TupleFactory factory(columns.second, all_columns);
   const Tuple under_test0a = factory.allocateOne(0);
   const Tuple under_test0b = factory.allocateOne(0);
   const Tuple under_test1 = factory.allocateOne(1);
   const Tuple under_test2 = factory.allocateOne(2);
   ASSERT_EQ(under_test0a, under_test0b);
   ASSERT_NE(under_test0a, under_test1);
   ASSERT_EQ(under_test0a, under_test2);
   ASSERT_NE(under_test0b, under_test1);
   ASSERT_EQ(under_test0b, under_test2);
   ASSERT_NE(under_test1, under_test2);
}

TEST(Tuple, comparesFieldsCorrectly) {
   auto columns = createSinglePartitionColumns();
   TupleFactory factory(columns.second, columns.second.metadata);
   const Tuple tuple0a = factory.allocateOne(0);
   const Tuple tuple0b = factory.allocateOne(0);
   const Tuple tuple1 = factory.allocateOne(1);
   const Tuple tuple2 = factory.allocateOne(2);

   std::vector<silo::query_engine::actions::OrderByField> order_by_fields;
   order_by_fields.push_back({"dummy_indexed_string_column", true});
   const Tuple::Comparator under_test =
      Tuple::getComparator(columns.second.metadata, order_by_fields, std::nullopt);

   ASSERT_FALSE(under_test(tuple0a, tuple0b));
   ASSERT_FALSE(under_test(tuple0b, tuple0a));

   ASSERT_TRUE(under_test(tuple0a, tuple1));
   ASSERT_FALSE(under_test(tuple1, tuple0a));

   ASSERT_FALSE(under_test(tuple0a, tuple2));
   ASSERT_FALSE(under_test(tuple2, tuple0a));

   ASSERT_TRUE(under_test(tuple0b, tuple1));
   ASSERT_FALSE(under_test(tuple1, tuple0b));

   ASSERT_FALSE(under_test(tuple0b, tuple2));
   ASSERT_FALSE(under_test(tuple2, tuple0b));

   ASSERT_FALSE(under_test(tuple1, tuple2));
   ASSERT_TRUE(under_test(tuple2, tuple1));

   order_by_fields.clear();
   const Tuple::Comparator under_test2 =
      Tuple::getComparator(columns.second.metadata, order_by_fields, std::nullopt);

   ASSERT_FALSE(under_test2(tuple0a, tuple0b));
   ASSERT_FALSE(under_test2(tuple0b, tuple0a));

   ASSERT_FALSE(under_test2(tuple0a, tuple1));
   ASSERT_FALSE(under_test2(tuple1, tuple0a));

   ASSERT_FALSE(under_test2(tuple0a, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple0a));

   ASSERT_FALSE(under_test2(tuple0b, tuple1));
   ASSERT_FALSE(under_test2(tuple1, tuple0b));

   ASSERT_FALSE(under_test2(tuple0b, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple0b));

   ASSERT_FALSE(under_test2(tuple1, tuple2));
   ASSERT_FALSE(under_test2(tuple2, tuple1));

   order_by_fields.push_back({"dummy_date_column", true});
   const Tuple::Comparator under_test3 =
      Tuple::getComparator(columns.second.metadata, order_by_fields, std::nullopt);

   ASSERT_FALSE(under_test3(tuple0a, tuple0b));
   ASSERT_FALSE(under_test3(tuple0b, tuple0a));

   ASSERT_FALSE(under_test3(tuple0a, tuple1));
   ASSERT_FALSE(under_test3(tuple1, tuple0a));

   ASSERT_FALSE(under_test3(tuple0a, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple0a));

   ASSERT_FALSE(under_test3(tuple0b, tuple1));
   ASSERT_FALSE(under_test3(tuple1, tuple0b));

   ASSERT_FALSE(under_test3(tuple0b, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple0b));

   ASSERT_FALSE(under_test3(tuple1, tuple2));
   ASSERT_FALSE(under_test3(tuple2, tuple1));

   order_by_fields.push_back({"dummy_string_column", false});
   const Tuple::Comparator under_test4 =
      Tuple::getComparator(columns.second.metadata, order_by_fields, std::nullopt);

   ASSERT_FALSE(under_test4(tuple0a, tuple0b));
   ASSERT_FALSE(under_test4(tuple0b, tuple0a));

   ASSERT_FALSE(under_test4(tuple0a, tuple1));
   ASSERT_TRUE(under_test4(tuple1, tuple0a));

   ASSERT_FALSE(under_test4(tuple0a, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple0a));

   ASSERT_FALSE(under_test4(tuple0b, tuple1));
   ASSERT_TRUE(under_test4(tuple1, tuple0b));

   ASSERT_FALSE(under_test4(tuple0b, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple0b));

   ASSERT_TRUE(under_test4(tuple1, tuple2));
   ASSERT_FALSE(under_test4(tuple2, tuple1));
}

// NOLINTEND(bugprone-unchecked-optional-access)
